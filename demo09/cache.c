#include "../helpers/file.c"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// 文件缓存结构
typedef struct {
  const char *filename;
  char *content;
  size_t length;
} FileCache;

// 全局文件缓存数组
static FileCache *file_cache = NULL;
static int cache_size = 0;
static pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

// 从缓存获取文件内容，如果不存在则读取并缓存
static char *get_file_content(const char *filename, size_t *length) {
  pthread_mutex_lock(&cache_mutex);

  // 查找缓存
  for (int i = 0; i < cache_size; i++) {
    if (strcmp(file_cache[i].filename, filename) == 0) {
      *length = file_cache[i].length;
      char *content = file_cache[i].content;
      pthread_mutex_unlock(&cache_mutex);
      return content;
    }
  }

  // 缓存中不存在，读取文件
  char *content = read_file_to_string(filename);
  if (content) {
    // 扩展缓存数组
    file_cache =
        (FileCache *)realloc(file_cache, (cache_size + 1) * sizeof(FileCache));
    if (file_cache) {
      file_cache[cache_size].filename = strdup(filename);
      file_cache[cache_size].content = content;
      file_cache[cache_size].length = strlen(content);
      *length = file_cache[cache_size].length;
      cache_size++;
    }
  }

  pthread_mutex_unlock(&cache_mutex);
  return content;
}

// 清理文件缓存
static void cleanup_file_cache() {
  pthread_mutex_lock(&cache_mutex);

  for (int i = 0; i < cache_size; i++) {
    free((void *)file_cache[i].filename);
    free(file_cache[i].content);
  }

  free(file_cache);
  file_cache = NULL;
  cache_size = 0;

  pthread_mutex_unlock(&cache_mutex);
  pthread_mutex_destroy(&cache_mutex);
}