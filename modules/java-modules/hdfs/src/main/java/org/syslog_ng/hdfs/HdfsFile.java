/*
 * Copyright (c) 2017 Balabit
 * Copyright (c) 2017 Zoltan Pallagi <zoltan.pallagi@balabit.com>
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

import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.Path;

import java.io.IOException;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

public class HdfsFile {
    private FSDataOutputStream fsDataOutputStream;
    private Path path;
    private Date lastWrite;
    private Timer timeReap;

    public HdfsFile(Path filepath, FSDataOutputStream outputstream) {
        this.path = filepath;
        this.fsDataOutputStream = outputstream;
        this.lastWrite = null;
        this.timeReap = null;
    }

    public Path getPath() {
        return path;
    }

    public boolean isOpen() {
        return fsDataOutputStream != null;
    }

    public void flush() throws IOException {
         if (!isOpen())
            return;

         fsDataOutputStream.hflush();
         fsDataOutputStream.hsync();
    }

    public void write(byte[] message) throws IOException {
         if (!isOpen())
            throw new IOException("File is not open: "+path);

         fsDataOutputStream.write(message);

         updateTimeReap();
         lastWrite = new Date();
    }

    private int TIME_REAP_DEFAULT = 10000;

    class TimeReap extends TimerTask {
        public TimeReap() {
        }

        private long timeSinceLastWrite() {
           Date now = new Date();

           return now.getTime() - lastWrite.getTime();
        }

        @Override
        public void run() {
           if (timeSinceLastWrite() < TIME_REAP_DEFAULT) {
              //TODO: reschedule
              System.out.println("There was a write after starting the timer, it should be rescheduled.");
              timeReap.cancel();
              timeReap = new Timer();
              timeReap.schedule(new TimeReap(), TIME_REAP_DEFAULT);
              return;
           }

           System.out.println("Close file" + path);

           //TODO: archive should be done also if needed
           //TODO: probably flush is not needed but what the hack
           try {
               flush();
               close();
               //TODO: it should also remove itself from the containnig file hashtable
           } catch (IOException e) {
               //TODO: proper print stacktrace
               System.out.println("Exception: " + e + "; but not much we could do about it.");
           } finally {
               timeReap.cancel(); 
               timeReap = null;
           }
        };
    }

    protected void updateTimeReap() {
         if (timeReap != null)
            return; //Timer is active reschedule could be done, when the timer expires


         timeReap = new Timer();
         timeReap.schedule(new TimeReap(), TIME_REAP_DEFAULT);
    }

    public void close() throws IOException {
         if (!isOpen())
            return;

          timeReap.cancel();
          fsDataOutputStream.close();
          fsDataOutputStream = null;
          lastWrite = null;
    }
}
