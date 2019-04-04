/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Define REPORT_RESULT and REPORT_RESULT_SYNC for using in test code
 */

#ifndef REPORT_RESULT_H_
#define REPORT_RESULT_H_

#include <stdio.h>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#ifndef EMTEST_PORT_NUMBER
#error "EMTEST_PORT_NUMBER not defined"
#endif

static void EMSCRIPTEN_KEEPALIVE _ReportResult(int result, int sync)
{
  // Only report one result per test, even if the test misbehaves and tries to report more.
  static int reported = 0;
  if (reported) {
    EM_ASM({ throw "too many reported results from " + ('' + window.location).substr(0, 80) + ", not sending " + $0 }, result);
    return;
  }
  reported = 1;

  printf("result: %d\n", result);
  EM_ASM({
    var xhr = new XMLHttpRequest();
    var result = $0;
    if (Module['pageThrewException']) result = 12345;
    xhr.open('GET', 'http://localhost:' + $2 + '/report_result?' + result + '|' + ('' + window.location).substr(0, 80), !$1);
    xhr.send();
    if (!Module['pageThrewException'] /* for easy debugging, don't close window on failure */) setTimeout(function() { window.close() }, 1000);
  }, result, sync, EMTEST_PORT_NUMBER);
}

#if __EMSCRIPTEN_PTHREADS__
  #include <emscripten/threading.h>
  #define REPORT_RESULT(result) emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VII, _ReportResult, (result), 0)
  #define REPORT_RESULT_SYNC(result) emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VII, _ReportResult, (result), 1)
#else
  #define REPORT_RESULT(result) _ReportResult((result), 0)
  #define REPORT_RESULT_SYNC(result) _ReportResult((result), 1)
#endif

#else

#include <stdlib.h>
#define REPORT_RESULT(result)       \
  do {                              \
    printf("result: %d\n", result); \
    exit(result);                   \
  }

#define REPORT_RESULT_SYNC REPORT_RESULT

#endif // __EMSCRIPTEN__

#endif // REPORT_RESULT_H_
