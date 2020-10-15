/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 *
 *  Sun Microsystems Inc., March, 2001
 *
 *
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.installer.task;

import java.util.EventListener;
import java.util.EventObject;
import java.util.concurrent.ThreadPoolExecutor;
import javax.swing.event.EventListenerList;

/**
 * Class to observe {@link ThreadPoolExecutor}s
 */
public class ThreadPoolObserver {
    private EventListenerList eventListenerList = null;
    private ThreadPoolExecutor[] threadPoolExecutors = null;

    private Thread thread = null;

    private boolean run = true;
    private boolean isStarted = false;
    private boolean isFinished = false;

    private int lastRunCompletedTaskCount = 0;

    private int taskCount = -1;

    /**
     * Constructor
     */
    public ThreadPoolObserver() {
        init();
    }

    /**
     * Constructor
     * @param threadPoolExecutor The {@link ThreadPoolExecutor} to be observed
     */
    public ThreadPoolObserver(ThreadPoolExecutor threadPoolExecutor) {
        this(new ThreadPoolExecutor[]{threadPoolExecutor});
    }

    /**
     * Constructor
     * @param threadPoolExecutors The array of {@link ThreadPoolExecutor}s to be observed
     */
    public ThreadPoolObserver(ThreadPoolExecutor[] threadPoolExecutors) {
        this.threadPoolExecutors = threadPoolExecutors;

        init();
    }

    private void init() {
        eventListenerList = new EventListenerList();
    }

    public void setThreadPoolExecutors(ThreadPoolExecutor threadPoolExecutor) {
        setThreadPoolExecutors(new ThreadPoolExecutor[]{threadPoolExecutor});
    }

    public void setThreadPoolExecutors(ThreadPoolExecutor[] threadPoolExecutors) {
        if (thread != null && thread.isAlive()) {
            shutDown();
        }
        
        this.threadPoolExecutors = threadPoolExecutors;
        lastRunCompletedTaskCount = 0;
        taskCount = -1;
    }

    /**
     * Starts the observation of the thread pool(s)
     */
    public void observe() {
        //TODO: does not work when pool will be empty (no tasks will be executed)
        if (thread == null || !thread.isAlive()) {
            run = true;
            isStarted = false;
            isFinished = false;
            
            thread = new Thread(new ObserverThread(), "ObserverThread");
            thread.start();
        }
    }

    /**
     * Stops the observation
     */
    public void shutDown() {
        this.run = false;
    }

    /**
     * Indicates whether the observaton has been started
     * @return true if the observation has been started, false otherwise
     */
    public boolean isStarted() {
        return isStarted;
    }

    /**
     * Indicates whether the observaton has been finished
     * @return true if the observation has been finished, false otherwise
     */
    public boolean isFinished() {
        return isFinished;
    }

    /**
     * Sets the overall sum of tasks. In case of this value has been set the
     * {@link ThreadPoolObserver#getLastRunTaskCount()} returns with this value
     * instead of the one counted from the current thread pools.
     * @param taskCount Sets the overall sum of tasks.
     */
    public void setTaskCount(int taskCount) {
        this.taskCount = taskCount;
    }

    /**
     * Adds a {@link ThreadPoolListener}.
     * @param threadPoolListener The {@link ThreadPoolListener} to be added.
     */
    public void addThreadPoolListener(ThreadPoolListener threadPoolListener) {
        eventListenerList.add(ThreadPoolListener.class, threadPoolListener);
    }

    /**
     * Removes the {@link ThreadPoolListener}.
     * @param threadPoolListener The {@link ThreadPoolListener} to be removed.
     */
    public void removeThreadPoolListener(ThreadPoolListener threadPoolListener) {
        eventListenerList.remove(ThreadPoolListener.class, threadPoolListener);
    }

    /**
     * Propagates the event to the listeners
     * @param type The type of the events
     *
     * @see ThreadPoolEvent
     */
    protected void fireThreadPoolEvent(int type) {
        // Guaranteed to return a non-null array
        Object[] listeners = eventListenerList.getListenerList();
        // Process the listeners last to first, notifying
        // those that are interested in this event
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i] == ThreadPoolListener.class) {
                ((ThreadPoolListener) listeners[i + 1]).threadPoolActionPerformed(new ThreadPoolEvent(this, type));
            }
        }
    }

    /**
     * Thread which observes the list of thread pools
     */
    private class ObserverThread implements Runnable {
        private int lastCompletedTaskCount = 0;
        private int lastTaskCount = 0;

        public void run() {
            do {
                try {
                    Thread.sleep(200);
                } catch (InterruptedException e) {
                }

                // A new executable has been added to the thread pool
                if (!isStarted && getPoolSize() > 0) {
                    isStarted = true;
                    fireThreadPoolEvent(ThreadPoolEvent.EVENT_THREAD_POOL_STARTED);
                }

                // New execution has been started
                if (isStarted && !isFinished && lastCompletedTaskCount < getCompletedTaskCount()) {
                    lastCompletedTaskCount = getCompletedTaskCount();
                    fireThreadPoolEvent(ThreadPoolEvent.EVENT_THREAD_POOL_UPDATED);
                }

                // New task has been added
                if (isStarted && !isFinished && lastTaskCount < getTaskCount()) {
                    lastTaskCount = getTaskCount();
                    fireThreadPoolEvent(ThreadPoolEvent.EVENT_THREAD_POOL_UPDATED);
                }

                // All of the executable has been finshed
                if (isStarted && !isFinished && areTasksReady()) {
                    isFinished = true;

                    /**
                     * Store the last run's value in order to use it during the next run.
                     */
                    lastRunCompletedTaskCount = getCompletedTaskCount();

                    fireThreadPoolEvent(ThreadPoolEvent.EVENT_THREAD_POOL_FINISHED);

                    run = false;
                }
            } while (run);
        }
    }

    public boolean areTasksReady() {
        if (taskCount > -1) {
            return getCompletedTaskCount() == taskCount;
        } else {
            return getCompletedTaskCount() == getTaskCount();
        }
    }

    /**
     * Returns with task count of the last run
     * @return Returns the sum of tasks
     * extracting sum of tasks from previous runs
     *
     * @see ThreadPoolObserver#getTaskCount()
     */
    public int getLastRunTaskCount() {
        if (taskCount > -1) {
            return taskCount;
        } else {
            return getTaskCount() - lastRunCompletedTaskCount;
        }
    }

    /**
     * Returns with completed task count of the last run
     * @return Returns the sum of completed tasks
     * extracting sum of completed tasks from previous runs
     *
     * @see ThreadPoolObserver#getCompletedTaskCount()
     */
    public int getLastRunCompletedTaskCount() {
        return getCompletedTaskCount() - lastRunCompletedTaskCount;
    }
    
    /**
     * Returns the sum of tasks
     * @return the sum of tasks
     */
    public int getTaskCount() {
        int count = 0;

        for (ThreadPoolExecutor tpe : threadPoolExecutors) {
            if (tpe != null && !tpe.isTerminated()) {
                count += tpe.getTaskCount();
            }
        }

        return count;
    }

    /**
     * Returns the sum of active tasks
     * @return the sum of active tasks
     */
    public int getActiveCount() {
        int count = 0;

        for (ThreadPoolExecutor tpe : threadPoolExecutors) {
            if (tpe != null && !tpe.isTerminated()) {
                count += tpe.getActiveCount();
            }
        }

        return count;
    }
    /**
     * Returns the sum of pool sizes
     * @return the sum of pool sizes
     */
    public int getPoolSize() {
        int count = 0;

        for (ThreadPoolExecutor tpe : threadPoolExecutors) {
            if (tpe != null && !tpe.isTerminated()) {
                count += tpe.getPoolSize();
            }
        }

        return count;
    }

    /**
     * Returns the sum of completed tasks
     * @return the sum of completed tasks
     */
    public int getCompletedTaskCount() {
        int count = 0;

        for (ThreadPoolExecutor tpe : threadPoolExecutors) {
            if (tpe != null && !tpe.isTerminated()) {
                count += tpe.getCompletedTaskCount();
            }
        }

        return count;
    }

    /**
     * Event fired by {@link ThreadPoolObserver}
     */
    public class ThreadPoolEvent extends EventObject {
        public static final int EVENT_THREAD_POOL_STARTED  = 0;
        public static final int EVENT_THREAD_POOL_UPDATED  = 1;
        public static final int EVENT_THREAD_POOL_FINISHED = 2;

        private int type = -1;

        /**
         * Constructor
         * @param source The source of the event. Usually the {@link ThreadPoolObserver}.
         */
        public ThreadPoolEvent(Object source) {
            this(source, EVENT_THREAD_POOL_UPDATED);
        }

        /**
         * Constructor
         * @param source The source of the event. Usually the {@link ThreadPoolObserver}.
         * @param type The type of the event
         */
        public ThreadPoolEvent(Object source, int type) {
            super(source);

            this.type = type;
        }

        /**
         * Returns with the type of the event.
         * @return The type of the event.
         */
        public int getType() {
            return type;
        }
    }

    /**
     * Listener interface for capturing {@link ThreadPoolEvent}s
     */
    public interface ThreadPoolListener extends EventListener {
        /**
         * Implement method to capture {@link ThreadPoolEvent}s
         * @param threadPoolEvent The {@link ThreadPoolEvent} has been fired.
         */
        public void threadPoolActionPerformed(ThreadPoolEvent threadPoolEvent);
    }
}
