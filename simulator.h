#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdio.h>

// Constants
#define MAX_PROCESSES 50
#define MAX_NO_FILES 50
#define MAX_FILENAME_LEN 50
#define MAX_COMMANDS_PER_PROGRAM 100
#define MAX_COMMAND_LENGTH 100

// struct to store the partition numbers, space, and code (e.g., free, init, or program name)
typedef struct
{
    unsigned int partition_no;
    unsigned int size_of_partition;
    char partition_code[100];
} Mem_partitions;

// struct for PCB table implementation
typedef struct
{
    unsigned int pid;
    unsigned int parent_pid;
    char prog_name[100];
    unsigned int remaining_cpu_time;
    unsigned int partition_no;
    char process_state[50];
    char io_info[100];
} PCB;

// struct for external files
typedef struct
{
    char program_name[50];
    unsigned int program_size;
    unsigned int num_commands;
    char commands[MAX_COMMANDS_PER_PROGRAM][MAX_COMMAND_LENGTH];
} External_files;

// global variables
extern int current_pid;       // Global variable to keep track of the current process
extern int current_pcb_index; // to help readily access init process
extern Mem_partitions memory_partitions[6];
extern PCB pcb_table[MAX_PROCESSES];
extern External_files external_files[MAX_NO_FILES];

// variables for process mgt; fork and exec
extern int next_pid;
extern int process_count;

// global variables for program execution
extern unsigned int program_size;
extern unsigned int partition_size;
extern int partition_index;
extern unsigned int partition_number;

// Function prototypes

// initialization functions
void initialize_pcb_table();
void load_external_files(const char *external_filename, int trace_number);
void load_program_files(const char *filename, int file_index);

// helper Functions
unsigned int get_program_size(const char *program_name);
int find_suitable_partition(unsigned int program_size);

// process mgt functions
void fork_process(FILE *output_txt_file, int curr_time, int parent_pid);
int exec_program(FILE *output_txt_file, int curr_time, int child_pid, const char *program_name, unsigned int program_size, int partition_index,
                 char vector_table[25][7]);

// Scheduler
void routine_scheduler(FILE *output_txt_file, int *curr_time, int scheduler_time);

// system_status
void save_sys_status(int curr_time, const char *status_filename);

// ISR Activities
int isractivities(FILE *output_txt_file, char *curr_command, char *program_name,
                  int curr_command_duration, int curr_time, int vector_num,
                  char *vector_memory_loc, char vector_table[25][7], const char *status_filename);

// execute commands specific to programs
int execute_program_commands(FILE *output_txt_file, int curr_time, int pid, char vector_table[25][7], const char *status_filename);

#endif
