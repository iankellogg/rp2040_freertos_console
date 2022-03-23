

#include <FreeRTOS.h>
#include <task.h>
#include <log/log.h>
#include <console/console.h>
#include <console/linenoise.h>
#include <console/argtable3.h>
#include "app_console.h"
#include <errno.h>
#include <string.h>

static void initConsole()
{
	// Initalize UART

    /* Initialize the console */
    console_config_t console_config = {
            .max_cmdline_args = 8,
            .max_cmdline_length = 80,
            .hint_color = atoi(LOG_COLOR_CYAN)
    };
    console_init(&console_config);

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(10);
}

#include <sys/fcntl.h>
#define lineBufferSize 80
static char lineBuffer[lineBufferSize];

static void ConsoleTask(void *params)
{
	initConsole();

    //register_system();
    //register_nvs();
    //register_UltraVisc_Cmds();
    console_register_help_command();
    vTaskDelay(1000);

	// setvbuf(stream,NULL,_IONBF,0);
	// stdout = stream;
	// stdin = stream;
	// fcntl(fileno(stream), F_SETFL, O_NONBLOCK) ;

    const char* prompt = LOG_COLOR_I "CMD> " LOG_RESET_COLOR;

	setvbuf(stdin,NULL,_IONBF,0);
	setvbuf(stdout,NULL,_IONBF,0);

	linenoiseSetLineBuffer(lineBuffer,lineBufferSize);

    /* Figure out if the terminal supports escape sequences */
    int probe_status = linenoiseProbe();
    if (probe_status!=0) {
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Teraterm instead.\n");
        linenoiseSetDumbMode(1);

    }
    char *line;



    /* Main loop */
    while(1) {
//        int probe_status = linenoiseProbe();
//        if (probe_status!=0) {
//            linenoiseSetDumbMode(1);
//        } else
//        {
//            linenoiseSetDumbMode(0);
//        }
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char* line = linenoise(prompt);
        if (line == NULL) { /* Ignore empty lines */
            continue;
        }
        /* Add the command to the history */
        linenoiseHistoryAdd(line);

        /* Try to run the command */
        int ret;
        int err = console_run(line, &ret);
        if (err == ENOENT) {
            printf("Unrecognized command\n");
        } else if (err == EINVAL) {
            // command was empty
        } else if (err == 0 && ret != 0) {
        	//printf((char*)ret);
        } else if (err != 0) {
            printf("Internal error: %s\n",err);
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }
}

	#define ConsoleTaskSize	512
    static StaticTask_t xConsoleTaskBuffer;
    static StackType_t xConsoleStack[ ConsoleTaskSize ];

void Init_ConsoleTask()
{
	xTaskCreateStatic( ConsoleTask, "ConsoleTask",  ConsoleTaskSize,   NULL,    tskIDLE_PRIORITY, xConsoleStack,   &xConsoleTaskBuffer );
}
