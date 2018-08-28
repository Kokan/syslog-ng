/*
 * Copyright (c) 2015 Balabit
 * Copyright (c) 2015 Zoltan Pallagi <zoltan.pallagi@balabit.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

package org.syslog_ng.hdfs;

import org.syslog_ng.*;
import org.syslog_ng.options.InvalidOptionException;
import org.syslog_ng.logging.SyslogNgInternalLogger;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.security.UserGroupInformation;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.net.URI;
import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.UUID;

import java.util.Timer;
import java.util.TimerTask;

public class HdfsDestination extends StructuredLogDestination {
    private static final String LOG_TAG = "HDFS:";
    private static final String HADOOP_SECURITY_AUTH_KEY = "hadoop.security.authentication";
    private static final String DFS_SUPPORT_APPEND_KEY = "dfs.support.append";

    HdfsOptions options;
    Logger logger;

    private FileSystem hdfs;

    private boolean isOpened;
    private int maxFileNameLength = 255;
    private boolean appendEnabled;
    private String archiveDir;

    HashMap<String, HdfsFile> openedFiles;

    private Timer timeReap;

    private int TIME_REAP_DEFAULT = 10000;

    class TimeReap extends TimerTask {
        private Map<String, HdfsFile> files;
        public TimeReap(Map<String, HdfsFile> files) {
            this.files = files;
        }

        private boolean closeFile(HdfsFile hdfsfile) {
           System.out.println("Close file" + hdfsfile.getPath());

           try {
               hdfsfile.flush();
               hdfsfile.close();
               //TODO: it should also remove itself from the containnig file hashtable
           } catch (IOException e) {
               printStackTrace(e);
               return false;
           }

           archiveFile(hdfsfile);

           return true;
        }

        @Override
        public void run() {
           long nextTimerExpires = -1;
           for (Map.Entry<String, HdfsFile> entry : files.entrySet()) {
               long current = entry.getValue().timeSinceLastWrite();
               if (current < nextTimerExpires || nextTimerExpires == -1) {
                  nextTimerExpires = current;
               }

               if (current >= TIME_REAP_DEFAULT) //Expired
                  closeFile(entry.getValue());
           }

           if (nextTimerExpires < TIME_REAP_DEFAULT) {
              timeReap.cancel();
              timeReap = new Timer();
              timeReap.schedule(new TimeReap(files), nextTimerExpires);
              return;
           }

        };
    }

    public HdfsDestination(long handle) {
        super(handle);
        logger = Logger.getRootLogger();
        SyslogNgInternalLogger.register(logger);
        options = new HdfsOptions(this);
    }

    @Override
    public boolean init() {
        try {
            options.init();
        } catch (InvalidOptionException e) {
            logger.error(e.getMessage());
            return false;
        }
        appendEnabled = options.getAppendEnabled();
        archiveDir = options.getArchiveDir();
        if(appendEnabled && archiveDir != null) {
            logger.warn("Archiving is not allowed when append-enabled() is configured. "
                        + "Archiving disabled.");
            archiveDir = null;
        }
        return true;
    }

    @Override
    public boolean isOpened() {
        return isOpened;
    }

    @Override
    public boolean open() {
        logger.debug("Opening hdfs");
        openedFiles = new HashMap<String, HdfsFile>();
        isOpened = false;
        try {
            Configuration configuration = new Configuration();
            loadConfigResources(configuration);
            if (isKerberosAuthEnabled()) {
                setKerberosAuth(configuration);
            }
            if (appendEnabled) {
                enableDfsSupportAppend(configuration);
            }

            hdfs = FileSystem.get(new URI(options.getUri()), configuration);
            hdfs.getStatus(); // Just to be sure we are really connected to hdfs
            isOpened = true;
        } catch (Exception e) {
            printStackTrace(e);
            closeAll(false);
        }
        return isOpened;
    }

    private void loadConfigResources(Configuration configuration) {
        String[] resources = options.getResouces();
        if (resources == null) {
            return;
        }
        for (int i = 0; i < resources.length; i++) {
            String resource = resources[i];
            configuration.addResource(new Path(resource));
            logger.debug(String.format("Resource loaded: %s", resource));
        }
    }

    private boolean isKerberosAuthEnabled() {
        return options.getKerberosPrincipal() != null && options.getKerberosKeytabFile() != null;
    }

    private void setKerberosAuth(Configuration configuration) throws IOException {
        configuration.set(HADOOP_SECURITY_AUTH_KEY, "kerberos");
        UserGroupInformation.setConfiguration(configuration);
        UserGroupInformation.loginUserFromKeytab(options.getKerberosPrincipal(), options.getKerberosKeytabFile());
    }


    protected void updateFilesTimer(HdfsFile hdfsfile) {
        if (timeReap != null)
           return;

        timeReap = new Timer();
        timeReap.schedule( new TimeReap(openedFiles), TIME_REAP_DEFAULT);
    }

    @Override
    public boolean send(LogMessage logMessage) {
        isOpened = false;
        String resolvedFileName = options.getFileNameTemplate().getResolvedString(logMessage);
        HdfsFile hdfsfile = getHdfsFile(resolvedFileName);
        if (hdfsfile == null) {
            // Unable to open file
            closeAll(true);
            return false;
        }

        try {
            String formattedMessage = options.getTemplate().getResolvedString(logMessage);

            logger.debug("Outgoing message: " + formattedMessage);

            byte[] rawMessage = formattedMessage.getBytes(Charset.forName("UTF-8"));

            hdfsfile.write(rawMessage);
        } catch (IOException e) {
            printStackTrace(e);
            closeAll(false);
            return false;
        }

        updateFilesTimer(hdfsfile);

        isOpened = true;
        return true;
    }

    private HdfsFile getHdfsFile(String resolvedFileName) {
        HdfsFile hdfsFile = openedFiles.get(resolvedFileName);
        if (hdfsFile == null) {
            hdfsFile = createHdfsFile(resolvedFileName);
            openedFiles.put(resolvedFileName, hdfsFile);
        }
        if (!hdfsFile.isOpen()) {
            hdfsFile = createHdfsFile(resolvedFileName);
            openedFiles.put(resolvedFileName, hdfsFile);
        }
        return hdfsFile;
    }

    private HdfsFile createHdfsFile(String resolvedFileName) {
        Path filePath = getFilePath(resolvedFileName);
        FSDataOutputStream fsDataOutputStream = createFsDataOutputStream(hdfs, filePath);

        HdfsFile hdfsFile = new HdfsFile(filePath, fsDataOutputStream);

        return hdfsFile;
    }

    private Path getFilePath(String resolvedFileName) {
        if (!appendEnabled) {
            // when append is not enabled, we create a unique filename for every file
            resolvedFileName = String.format("%s.%s", resolvedFileName, UUID.randomUUID());
        }
        Path filePath = new Path(String.format("%s/%s", options.getUri(), resolvedFileName));

        if (filePath.getName().length() > options.getMaxFilenameLength()) {
            String fileName = truncateFileName(filePath.getName(), options.getMaxFilenameLength());
            logger.debug(String.format("Maximum file name length (%s) exceeded, truncated to %s", maxFileNameLength,
                    fileName));
            filePath = new Path(filePath.getParent(), fileName);
        }
        return filePath;
    }

    private String truncateFileName(String fileName, int maxLength) {
        if (fileName.length() > maxLength) {
            return fileName.substring(0, maxLength);
        }
        return fileName;
    }

    private FSDataOutputStream createFsDataOutputStream(FileSystem hdfs, Path file) {
        FSDataOutputStream fsDataOutputStream = null;
        try {
            hdfs.mkdirs(file.getParent());
            if (appendEnabled && hdfs.exists(file)) {
                logger.debug(String.format("Appending file %s", file.toString()));
                fsDataOutputStream = hdfs.append(file);
            } else {
                logger.debug(String.format("Creating file %s", file.toString()));
                fsDataOutputStream = hdfs.create(file);
            }
        } catch (IOException e) {
            printStackTrace(e);
        }
        return fsDataOutputStream;
    }

    private void enableDfsSupportAppend(Configuration configuration) {
        configuration.setBoolean(DFS_SUPPORT_APPEND_KEY, true);
    }

    @Override
    public void onMessageQueueEmpty() {
        /*
         * Trying to flush however the effect is questionable... (It is not guaranteed
         * that data has been flushed to persistent store on the datanode)
         */

        logger.debug("Flushing hdfs");
        for (Map.Entry<String, HdfsFile> entry : openedFiles.entrySet()) {
            HdfsFile hdfsfile = entry.getValue();
            try {
                hdfsfile.flush();
            } catch (IOException e) {
                logger.debug(String.format("Flush failed on file %s, reason: %s", entry.getKey(), e.getMessage()));
            }
        }
    }

    @Override
    public void close() {
        logger.debug("Closing hdfs");

        closeAll(true);
        isOpened = false;
    }

    private void closeAll(boolean isArchiving) {
        if (timeReap != null) {
            timeReap.cancel();
            timeReap = null;
        }

        closeAllHdfsFiles();

        if (isArchiving) {
            archiveFiles();
        }

        closeHdfs();
        isOpened = false;
    }

    private void closeAllHdfsFiles() {
        for (Map.Entry<String, HdfsFile> entry : openedFiles.entrySet()) {
            HdfsFile hdfsfile = entry.getValue();
            try {
                logger.debug(String.format("Closing file: %s", entry.getKey()));
                hdfsfile.close();
            } catch (IOException e) {
                printStackTrace(e);
            }
        }
    }

    private void closeHdfs() {
        if (hdfs == null)
           return;

        try {
            hdfs.close();
        } catch (IOException e) {
            printStackTrace(e);
        }
        finally {
            hdfs = null;
        }
    }

    private void archiveFile(HdfsFile hdfsfile) {
        Path filePath = hdfsfile.getPath();
        logger.debug(String.format("Trying to archive %s to %s", filePath.getName(), options.getArchiveDir()));
        Path archiveDirPath = new Path(String.format("%s/%s", options.getUri(), archiveDir));

        try {
           hdfs.mkdirs(archiveDirPath);
           hdfs.rename(filePath, archiveDirPath);
        } catch (IOException e) {
            logger.debug(String.format("Unable to archive, reason: %s", e.getMessage()));
        }
    }

    private void archiveFiles() {
        if (archiveDir == null)
           return;

        for (Map.Entry<String, HdfsFile> entry : openedFiles.entrySet()) {
            archiveFile( entry.getValue() );
        }
    }

    private void printStackTrace(Throwable e) {
        String trace = org.apache.commons.lang.exception.ExceptionUtils.getStackTrace(e);
        logger.error(trace);
    }

    @Override
    public String getNameByUniqOptions() {
        return String.format("hdfs,%s,%s", options.getUri(), options.getFileNameTemplate().getValue());
    }

    @Override
    public void deinit() {
        logger.debug("Deinitialize hdfs destination");

        options.deinit();
    }

}
