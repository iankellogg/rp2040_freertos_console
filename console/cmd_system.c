/* Console example — various system commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <log/log.h>
#include "console/console.h"
#include "console/argtable3.h"
#include <FreeRTOS.h>
#include <task.h>
#include "console/cmd_system.h"
//#include "cmsis_device.h"
#include <errno.h>

static const char *TAG = "cmd_system";



/** 'restart' command restarts the program */

static int restart(int argc, char **argv)
{
    LOGI(TAG, "Restarting");
    vTaskDelay(10);
   // NVIC_SystemReset();
    return 0;
}

static void register_restart()
{
    const console_cmd_t cmd = {
        .command = "restart",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &restart,
    };
    assert( console_cmd_register(&cmd) ==0);
}

/** 'free' command prints available heap memory */

static int free_mem(int argc, char **argv)
{
    printf("%d\n", xPortGetFreeHeapSize());
    return 0;
}

static void register_free()
{
    const console_cmd_t cmd = {
        .command = "free",
        .help = "Get the current size of free heap memory",
        .hint = NULL,
        .func = &free_mem,
    };
    assert( console_cmd_register(&cmd)==0 );
}

/* 'heap' command prints minumum heap size */
//static int heap_size(int argc, char **argv)
//{
//    uint32_t heap_size = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
//    ESP_LOGI(TAG, "min heap size: %u", heap_size);
//    return 0;
//}
//
//static void register_heap()
//{
//    const esp_console_cmd_t heap_cmd = {
//        .command = "heap",
//        .help = "Get minimum size of free heap memory that was available during program execution",
//        .hint = NULL,
//        .func = &heap_size,
//    };
//    ESP_ERROR_CHECK( esp_console_cmd_register(&heap_cmd) );
//
//}

/** 'tasks' command prints the list of tasks and related information */
#if configUSE_STATS_FORMATTING_FUNCTIONS==1
static int tasks_info(int argc, char **argv)
{
    const size_t bytes_per_task = 400; /* see vTaskList description */
    char *task_list_buffer = (char*)malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
    if (task_list_buffer == NULL) {
        LOGE(TAG, "failed to allocate buffer for vTaskList output");
        return 1;
    }
    fputs("Task Name\tStatus\tPrio\tHWM\tTask#\n", stdout);
    vTaskList(task_list_buffer);
    fputs(task_list_buffer, stdout);
    free(task_list_buffer);
    return 0;
}

static void register_tasks()
{
    const console_cmd_t cmd = {
        .command = "tasks",
        .help = "Get information about running tasks",
        .hint = NULL,
        .func = &tasks_info,
    };
    assert( console_cmd_register(&cmd)==0 );
}
#endif

/***************** SetDebugLevel ***************/
static struct {
    struct arg_str *tag;
    struct arg_int *level;
    struct arg_end *end;
} SetDebugLevel_args;

static int cmd_SetDebugLevel(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &SetDebugLevel_args);
    if (nerrors != 0) {
        arg_print_errors(stdout, SetDebugLevel_args.end, argv[0]);
        return EINVAL;
    }
    // Bound Check
	char *tag =SetDebugLevel_args.tag->sval[0];
	int16_t level=SetDebugLevel_args.level->ival[0];
	char *sLevel = "NONE";
	switch (level)
	{
		case LOG_NONE:
			sLevel="NONE";
			break;
		case LOG_ERROR:
			sLevel="ERROR";
			break;
		case LOG_WARN:
			sLevel="WARN";
			break;
		case LOG_INFO:
			sLevel="INFO";
			break;
		case LOG_DEBUG:
			sLevel="DEBUG";
			break;
		case LOG_VERBOSE:
			sLevel="VERBOSE";
			break;
	}
    printf( "Setting Debug Level for %s to %s\r\n",tag,sLevel);
	log_level_set(tag,level);
    return 0;

}
static void register_SetDebugLevel(void)
{
	/*********** Set Sensor Drive *******************/
	SetDebugLevel_args.tag = arg_strn (NULL,NULL,"Tag",1,1,"Debug tag used" "");
	SetDebugLevel_args.level = arg_intn (NULL,NULL,"Level",1,1,"Log Severity" "None: 0, Error: 1, Warn: 2, Info: 3, Debug: 4, Verbose: 5");
	SetDebugLevel_args.end = arg_end(2);

    const console_cmd_t cmd = {
        .command = "SetDebugLevel",
        .help = "Sets DC constant Current for Opto-Transistors ",
        .hint = NULL,
        .func = &cmd_SetDebugLevel,
        .argtable = &SetDebugLevel_args
    };
    assert( console_cmd_register(&cmd) == 0);
}
void register_system()
{
    register_free();
   // register_heap();
    register_restart();
    register_SetDebugLevel();
#if configUSE_STATS_FORMATTING_FUNCTIONS==1
    register_tasks();
#endif
}

