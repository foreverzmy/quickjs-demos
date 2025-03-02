#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *testurl = "https://jsonplaceholder.typicode.com/todos/1";

typedef struct {
  char *data;
  size_t size;
} ResponseData;

// Add the write callback function
static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp) {
  size_t realsize = size * nmemb;
  ResponseData *resp = (ResponseData *)userp;

  // Check for overflow
  if (realsize > SIZE_MAX - resp->size - 1) {
    fprintf(stderr, "Memory size overflow\n");
    return 0;
  }

  char *ptr = realloc(resp->data, resp->size + realsize + 1);
  if (!ptr) {
    fprintf(stderr, "Memory reallocation failed\n");
    return 0;
  }

  resp->data = ptr;
  memcpy(&(resp->data[resp->size]), contents, realsize);
  resp->size += realsize;
  resp->data[resp->size] = '\0';

  return realsize;
}

int main() {
  CURL *curl;
  CURLcode res;
  // Initialize response data with enough space
  ResponseData response_data = {.data = malloc(1), .size = 0};
  if (!response_data.data) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1;
  }
  response_data.data[0] = '\0'; // Ensure null termination

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, testurl);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response_data);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "QuickJS-Fetch/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // 30 秒超时

    // Perform the request
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    } else {
      // Print the response data
      printf("Response: %s\n", response_data.data);
    }
  }

  // Clean up
  curl_easy_cleanup(curl);

  // Free memory only if it was allocated
  if (response_data.data) {
    free(response_data.data);
    response_data.data = NULL;
  }

  return 0;
}
