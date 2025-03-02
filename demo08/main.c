#include "../helpers/console.c"
#include "../helpers/exception.c"
#include "../helpers/file.c"
#include "../quickjs/quickjs.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// 任务结构体
typedef struct {
  const char *filename;
  int iterations;
  double execution_time;
  int task_id;
} Task;

// 任务队列节点
typedef struct TaskNode {
  Task task;
  struct TaskNode *next;
} TaskNode;

// 任务队列
typedef struct {
  TaskNode *head;
  TaskNode *tail;
  int size;
  pthread_mutex_t mutex;
  pthread_cond_t not_empty;
} TaskQueue;

typedef struct {
  int task_id;
  double execution_time;
} TaskExecutionTime;

// 线程池
typedef struct {
  pthread_t *threads;
  int thread_count;
  TaskQueue queue;
  int shutdown;
  pthread_mutex_t shutdown_mutex;
  int completed_tasks;
  pthread_mutex_t completed_mutex;
  pthread_cond_t all_completed;

  TaskExecutionTime *task_execution_times;
} ThreadPool;

// 线程数据
typedef struct {
  ThreadPool *pool;
  int thread_id;
  JSRuntime *runtime;
} ThreadData;

// 初始化任务队列
void init_task_queue(TaskQueue *queue) {
  queue->head = NULL;
  queue->tail = NULL;
  queue->size = 0;
  pthread_mutex_init(&queue->mutex, NULL);
  pthread_cond_init(&queue->not_empty, NULL);
}

// 添加任务到队列
void enqueue_task(TaskQueue *queue, Task task) {
  TaskNode *node = (TaskNode *)malloc(sizeof(TaskNode));
  node->task = task;
  node->next = NULL;

  pthread_mutex_lock(&queue->mutex);

  if (queue->tail == NULL) {
    queue->head = node;
    queue->tail = node;
  } else {
    queue->tail->next = node;
    queue->tail = node;
  }

  queue->size++;
  pthread_cond_signal(&queue->not_empty);
  pthread_mutex_unlock(&queue->mutex);
}

// 从队列中获取任务
int dequeue_task(TaskQueue *queue, Task *task, int *shutdown_flag) {
  pthread_mutex_lock(&queue->mutex);

  // 当队列为空且没有关闭信号时等待
  // while (queue->size == 0 && !(*shutdown_flag)) {
  //   pthread_cond_wait(&queue->not_empty, &queue->mutex);
  // }

  // 如果收到关闭信号且队列为空，返回0表示没有任务
  if (queue->size == 0 || *shutdown_flag) {
    pthread_mutex_unlock(&queue->mutex);
    return 0;
  }

  if (queue->head == NULL) {
    pthread_mutex_unlock(&queue->mutex);
    return 0;
  }

  TaskNode *node = queue->head;
  *task = node->task;

  queue->head = node->next;
  if (queue->head == NULL) {
    queue->tail = NULL;
  }

  queue->size--;
  pthread_mutex_unlock(&queue->mutex);

  free(node);
  return 1;
}

// 销毁任务队列
void destroy_task_queue(TaskQueue *queue) {
  pthread_mutex_lock(&queue->mutex);

  TaskNode *current = queue->head;
  while (current != NULL) {
    TaskNode *next = current->next;
    free(current);
    current = next;
  }

  queue->head = NULL;
  queue->tail = NULL;
  queue->size = 0;

  pthread_mutex_unlock(&queue->mutex);
  pthread_mutex_destroy(&queue->mutex);
  pthread_cond_destroy(&queue->not_empty);
}

// Function to evaluate a JS file with QuickJS
static int eval_file(JSContext *ctx, const char *filename) {
  char *js_code = read_file_to_string(filename);

  // Evaluate JS code
  JSValue val =
      JS_Eval(ctx, js_code, strlen(js_code), filename, JS_EVAL_TYPE_GLOBAL);
  free(js_code);

  if (JS_IsException(val)) {
    check_and_print_exception(ctx);
    return 1;
  }
  JS_FreeValue(ctx, val);
  return 0;
}

// 执行任务
void execute_task(JSRuntime *runtime, Task *task) {
  clock_t start, end;
  start = clock();

  // 为任务创建新的 JSContext
  JSContext *ctx = JS_NewContext(runtime);
  if (!ctx) {
    fprintf(stderr, "Failed to create JS context for task %d\n", task->task_id);
    return;
  }

  js_std_init_console(ctx);

  // 执行指定次数的迭代
  for (int i = 0; i < task->iterations; i++) {
    if (eval_file(ctx, task->filename) < 0) {
      fprintf(stderr, "Error executing %s in task %d\n", task->filename,
              task->task_id);
      break;
    }
  }

  // 清理 JSContext
  JS_FreeContext(ctx);
  JS_RunGC(JS_GetRuntime(ctx));

  end = clock();
  task->execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;
}

// 线程工作函数
void *worker_thread(void *arg) {
  ThreadData *thread_data = (ThreadData *)arg;
  ThreadPool *pool = thread_data->pool;
  int thread_id = thread_data->thread_id;

  // 每个线程创建自己的 JSRuntime
  JSRuntime *runtime = JS_NewRuntime();
  if (!runtime) {
    fprintf(stderr, "Failed to create JS runtime for thread %d\n", thread_id);
    return NULL;
  }

  thread_data->runtime = runtime;

  printf("Thread %d started with its own JSRuntime\n", thread_id);

  // 循环处理任务
  while (1) {
    // 检查是否需要关闭线程
    pthread_mutex_lock(&pool->shutdown_mutex);
    int shutdown = pool->shutdown;
    pthread_mutex_unlock(&pool->shutdown_mutex);

    // 获取任务
    Task task;
    int got_task = dequeue_task(&pool->queue, &task, &shutdown);
    // 如果没有任务且收到关闭信号，退出循环
    if (!got_task && shutdown) {
      break;
    }

    if (got_task) {
      // printf("Thread %d executing task %d: %s (%d iterations)\n", thread_id,
      //        task.task_id, task.filename, task.iterations);

      // 执行任务
      execute_task(runtime, &task);

      // printf("Thread %d completed task %d in %.6f seconds\n", thread_id,
      //        task.task_id, task.execution_time);

      // 将任务结果存储到主线程的任务数组中
      pthread_mutex_lock(&pool->completed_mutex);
      pool->task_execution_times[task.task_id - 1].task_id = task.task_id;
      pool->task_execution_times[task.task_id - 1].execution_time =
          task.execution_time;
      pool->completed_tasks++;
      // 打印任务结果
      // printf("%-20s | %-15d | %-15.6f\n", task.filename, task.iterations,
      //        task.execution_time);
      pthread_mutex_unlock(&pool->completed_mutex);
    } else {
      printf("Thread %d got none task, completed %d tasks\n", thread_id,
             pool->completed_tasks);
      // 如果没有新任务了，发出信号
      pthread_mutex_lock(&pool->completed_mutex);
      pthread_cond_signal(&pool->all_completed);
      pthread_mutex_unlock(&pool->completed_mutex);

      break;
    }
  }

  // 清理 JSRuntime
  JS_FreeRuntime(runtime);
  printf("Thread %d shutting down\n", thread_id);

  return NULL;
}

// 初始化线程池
ThreadPool *init_thread_pool(int thread_count, int task_count) {
  ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
  if (!pool) {
    fprintf(stderr, "Failed to allocate memory for thread pool\n");
    return NULL;
  }

  pool->thread_count = thread_count;
  pool->threads = (pthread_t *)malloc(thread_count * sizeof(pthread_t));
  pool->shutdown = 0;
  pool->completed_tasks = 0;
  pool->task_execution_times =
      (TaskExecutionTime *)calloc(task_count, sizeof(TaskExecutionTime));

  pthread_mutex_init(&pool->shutdown_mutex, NULL);
  pthread_mutex_init(&pool->completed_mutex, NULL);
  pthread_cond_init(&pool->all_completed, NULL);

  init_task_queue(&pool->queue);

  // 创建线程数据
  ThreadData *thread_data =
      (ThreadData *)malloc(thread_count * sizeof(ThreadData));

  // 创建工作线程
  for (int i = 0; i < thread_count; i++) {
    thread_data[i].pool = pool;
    thread_data[i].thread_id = i;
    thread_data[i].runtime = NULL;

    if (pthread_create(&pool->threads[i], NULL, worker_thread,
                       &thread_data[i]) != 0) {
      fprintf(stderr, "Failed to create thread %d\n", i);
      // 清理已创建的线程
      for (int j = 0; j < i; j++) {
        pthread_cancel(pool->threads[j]);
        pthread_join(pool->threads[j], NULL);
      }

      free(pool->threads);
      free(thread_data);
      free(pool);
      return NULL;
    }
  }

  return pool;
}

// 关闭线程池
void shutdown_thread_pool(ThreadPool *pool) {
  if (!pool)
    return;

  // 设置关闭标志
  pthread_mutex_lock(&pool->shutdown_mutex);
  pool->shutdown = 1;
  pthread_mutex_unlock(&pool->shutdown_mutex);

  // 唤醒所有等待任务的线程
  pthread_mutex_lock(&pool->queue.mutex);
  pthread_cond_broadcast(&pool->queue.not_empty);
  pthread_mutex_unlock(&pool->queue.mutex);

  // 等待所有线程结束
  for (int i = 0; i < pool->thread_count; i++) {
    pthread_join(pool->threads[i], NULL);
  }

  // 清理资源
  destroy_task_queue(&pool->queue);
  pthread_mutex_destroy(&pool->shutdown_mutex);
  pthread_mutex_destroy(&pool->completed_mutex);
  pthread_cond_destroy(&pool->all_completed);

  free(pool->task_execution_times);
  free(pool->threads);
  free(pool);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <js_file1> [<js_file2> ...] <iterations>\n",
            argv[0]);
    return 1;
  }

  // 解析执行次数（最后一个参数）
  int iterations = atoi(argv[argc - 1]);
  if (iterations <= 0) {
    fprintf(stderr, "Invalid number of iterations: %s\n", argv[argc - 1]);
    return 1;
  }

  clock_t start, end;
  start = clock();

  // JS文件数量
  int num_files = argc - 2;
  // 任务数，总文件数乘以执行次数
  int total_tasks = num_files * iterations;

  // 创建线程池（线程数等于处理器核心数）
  int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

  printf("Creating thread pool with %d threads for %d tasks\n", num_cores,
         total_tasks);
  // 初始化线程池
  ThreadPool *pool = init_thread_pool(num_cores, total_tasks);

  if (!pool) {
    fprintf(stderr, "Failed to initialize thread pool\n");
    return 1;
  }

  // 创建任务数组用于存储结果
  Task *tasks = (Task *)malloc(total_tasks * sizeof(Task));

  // 添加任务到队列
  int task_id = 0;
  for (int i = 0; i < num_files; i++) {
    for (int j = 0; j < iterations; j++) {
      tasks[task_id].filename = argv[i + 1];
      tasks[task_id].iterations = 1; // 每个任务只执行一次
      tasks[task_id].execution_time = 0.0;
      tasks[task_id].task_id = task_id + 1;
      enqueue_task(&pool->queue, tasks[task_id]); // 添加任务到队列
      task_id++;
    }
  }

  printf("Added %d tasks to the queue\n", total_tasks);

  // 等待所有任务完成
  pthread_mutex_lock(&pool->completed_mutex);
  while (pool->completed_tasks < total_tasks) {
    pthread_cond_wait(&pool->all_completed, &pool->completed_mutex);
  }
  pthread_mutex_unlock(&pool->completed_mutex);

  // 打印结果
  printf("\nExecution Results:\n");
  printf("--------------------------------------------------\n");
  printf("%-20s | %-15s\n", "File", "Time (seconds)");
  printf("--------------------------------------------------\n");

  double *file_times = (double *)calloc(num_files, sizeof(double));
  // 累加每个文件的执行时间和迭代次数
  for (int i = 0; i < total_tasks; i++) {
    // 找出当前任务对应的文件索引
    int file_index = -1;
    for (int j = 0; j < num_files; j++) {
      if (strcmp(tasks[i].filename, argv[j + 1]) == 0) {
        file_index = j;
        break;
      }
    }

    if (file_index >= 0) {
      file_times[file_index] += pool->task_execution_times[i].execution_time;
    }
  }

  double total_time = 0.0;
  for (int i = 0; i < num_files; i++) {
    printf("%-20s | %-15.6f\n", argv[i + 1], file_times[i]);
    total_time += file_times[i];
  }

  printf("--------------------------------------------------\n");
  printf("Total execution time across all tasks: %.6f seconds\n", total_time);
  printf("Average execution time per task: %.6f seconds\n",
         total_time / num_files);

  end = clock();
  printf("Total execution time: %.6f seconds\n",
         ((double)(end - start)) / CLOCKS_PER_SEC);

  free(file_times);

  // 关闭线程池
  shutdown_thread_pool(pool);

  // 清理资源
  free(tasks);

  return 0;
}
