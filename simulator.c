/*
REUSING ASSIGNMENT 1 SOLUTION

SYSC 4001 Assignment 2 - Part 3
Created by: Ayra Mensah [101221911] & Sabeen Rafiq [101258923]
Group: L1 - 12
Submitted On: 2024-11-01
*/

// include statements
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> //for srand()

#include "interrupts_101258923_101221911.h"
#include <ctype.h>  //this helps later on in script when 'isspace' is used to check whitespaces. Referenced from geeksforgeeks.org
#include <limits.h> //for UINT_MAX

// global variables(these are 'extern' from within the header file)
// to keep track of current process/initialize pid,
// and memory loc int needed later in script
int current_pid = 0;
int current_pcb_index = 0;
int vector_memory_loc_int = 0;

// Global variables to help with program execution
unsigned int program_size = 0;
unsigned int partition_size = 0;
int partition_index = -1;
unsigned int partition_number = 0;

// initialize the memory partitions struct with the entries in the instructions
Mem_partitions memory_partitions[6] = {
    {1, 40, "unoccupied"},
    {2, 25, "unoccupied"},
    {3, 15, "unoccupied"},
    {4, 10, "unoccupied"},
    {5, 8, "unoccupied"},
    // pid 0 with 'init' will use the partition 6 and 1MB of memory
    {6, 2, "init"}};

// initialize pcb table for new processes
void initialize_pcb_table()
{
    pcb_table[0].pid = 0;
    pcb_table[0].parent_pid = -1;
    strcpy(pcb_table[0].prog_name, "init"); // when new process is initialize, it gets the init keyword
    pcb_table[0].remaining_cpu_time = 1;
    pcb_table[0].partition_no = 6;
    strcpy(pcb_table[0].process_state, "Ready");
    strcpy(pcb_table[0].io_info, "None");

    // Mark Partition 6 as occupied by "init"
    strcpy(memory_partitions[5].partition_code, "init"); // Index 5 corresponds to Partition 6
}

// initializing pcb table with size of MAX PROCESSES defined in header, & external files with MAX_NO_FILES
PCB pcb_table[MAX_PROCESSES];
External_files external_files[MAX_NO_FILES];

// initialization func to load commands from program files and basically grab its content
void load_program_files(const char *filename, int file_index)
{
    FILE *file = fopen(filename, "r"); // open and read
    if (file == NULL)                  // if not available, or issues with the file
    {
        printf("Error opening program file: %s\n", filename);
        return;
    }

    char line[100];
    int cmd_index = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        // removing trailing newline and carriage return characters
        int len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        {
            line[len - 1] = '\0';
            len--;
        }

        // trimming any leading whitespace
        char *trimmed_line = line;
        // this is where the isspace from the <ctype.h> library comes in handy
        while (isspace((unsigned char)*trimmed_line))
            trimmed_line++;

        // skipping any empty lines
        if (*trimmed_line == '\0')
            continue;

        // Store the command
        if (cmd_index < MAX_PROCESSES)
        {
            strcpy(external_files[file_index].commands[cmd_index], trimmed_line);
            cmd_index++;
        }
        else
        {
            printf("Error: Maximum number of commands exceeded for program %s\n", filename);
            break;
        }
    }

    fclose(file);

    // Store the number of commands loaded
    external_files[file_index].num_commands = cmd_index;

    // For debugging purposes, print the loaded program
    printf("Program name: %s, Program size: %u\n\n", external_files[file_index].program_name, external_files[file_index].program_size);
    printf("Number of commands loaded: %d\n", cmd_index);
    for (int i = 0; i < cmd_index; i++)
    {
        printf("Command %d: %s\n", i, external_files[file_index].commands[i]);
    }
    // throughout script, such print statements are just to help organize terminal output
    // to make it easy to follow the script and what it's doing
    printf("---------------------------------------------\n");
}

// Main function
// Take in arguments from terminal
int main(int agrc, char *argv[])
{
    // declare & initialize variables
    char curr_line[200];
    int curr_time = 0;
    int vector_num = 0;
    int vector_memory_loc_int = 0;
    char vector_memory_loc[4];
    char output_file[200];
    char status_filename[200];
    char external_filename[200];
    int trace_number;
    FILE *trace_txt_file;
    FILE *output_txt_file;
    FILE *status_txt_file;
    FILE *external_txt_file;

    // organizing terminal output in case you want to follow how the script is working
    printf("\n\nStarting script...\n");
    printf("---------------------------------------------\n\n");

    if (agrc < 2)
    {
        printf("Usage: %s trace1.txt\n", argv[0]);
        return 1;
    }

    // Trace file name is the 1st argument entered
    char *trace_txt_file_name = argv[1];
    // open the trace.txt file (the file that holds the commands) to read
    trace_txt_file = fopen(trace_txt_file_name, "r");

    // Print error if file does not open
    if (trace_txt_file == NULL)
    {
        printf("Error opening trace file");
        return 1;
    }

    // use scanf to get the required number from the input
    sscanf(trace_txt_file_name, "trace%d_101258923_101221911.txt", &trace_number);

    //  corresponding system_status file (using string format)
    sprintf(external_filename, "external_files%d_101258923_101221911.txt", trace_number);

    // printf("EXTERNAL FILENAME in main: %s\n", external_filename);

    // loading external files
    printf("Loading external files...\n");
    printf("---------------------------------------------\n");
    load_external_files(external_filename, trace_number);
    printf("\n\nLoaded external files!\n");
    printf("---------------------------------------------\n\n");

    //  corresponding system_status file (using string format)
    sprintf(status_filename, "system_status%d_101258923_101221911.txt", trace_number);

    // Clear the system_status.txt file at the start so we can later append to it as needed
    status_txt_file = fopen(status_filename, "w");

    if (status_filename == NULL)
    {
        printf("Error opening system_status file\n");
        return 1;
    }
    fclose(status_txt_file);

    // initialising PCB
    initialize_pcb_table();

    save_sys_status(0, status_filename);

    // Get Vector Table values from vector_table.txt
    FILE *vector_table_file;
    char vector_table[25][7];
    int i = 0;
    char curr_vec_line[200];

    // Open file in read mode
    vector_table_file = fopen("vector_table_101258923_101221911.txt", "r");

    // Print error if file does not open
    if (vector_table_file == NULL)
    {
        printf("Error opening vector table file");
        return 1;
    }
    // get all lines in file and store in vector_table & repeat until the end of the file
    // fgets: char *fgets(char *str, int n, FILE *stream); Referenced from www.geeksforgeeks.org
    while (fgets(curr_vec_line, sizeof(curr_vec_line), vector_table_file) != NULL)
    {
        // Store string (vector number in hex) in array at postion i
        // sscanf: int sscanf(const char *str, const char *format, ...); Referenced from www.tutorialspoint.com
        sscanf(curr_vec_line, "%s", vector_table[i]);
        // printf("Vector table entry %d: %s\n", i, vector_table[i]);
        //  increment index number
        i++;
    }

    // close vector table file
    fclose(vector_table_file);

    // printf("Trace number: %d\n", trace_number);

    // find corresponding execution file (using string format)
    sprintf(output_file, "execution%d_101258923_101221911.txt", trace_number);

    // open the file
    output_txt_file = fopen(output_file, "w");

    // Print error if file does not open
    if (output_txt_file == NULL)
    {
        printf("Error opening output file");
        return 1;
    }

    // Add Srand() to get a seed for rand() to different random values when needed
    //  void srand(unsigned int seed) Referenced from www.tutorialspoint.com
    srand((unsigned)time(NULL));

    // Repeat while the line still exists (have not reached end of file)
    //  fgets: char *fgets(char *str, int n, FILE *stream); Referenced from www.geeksforgeeks.org
    while (fgets(curr_line, sizeof(curr_line), trace_txt_file) != NULL)
    {
        // declare variables needed for each line (once sent to isractivites the values are not needed)
        char curr_command[100];
        char program_name[100] = "";
        int curr_command_duration;

        printf("\nProcessing line: %s\n", curr_line);
        curr_line[strcspn(curr_line, "\n")] = 0;

        // If SYSCALL or END_IO split into command and vector number / end_io number
        if ((strncmp(curr_line, "SYSCALL", 7) == 0) || (strncmp(curr_line, "END_IO", 6) == 0))
        {
            // take the line and split into curr_command, vector_numand curr_command_duration
            // sscanf: int sscanf(const char *str, const char *format, ...); Referenced from www.tutorialspoint.com
            sscanf(curr_line, "%s %d, %d", curr_command, &vector_num, &curr_command_duration);

            // Calculate Vector memory location vector number * 2 bytes
            vector_memory_loc_int = vector_num * 2;

            // Convert the integer to hexadecimal
            // int sprintf(char *str, const char *format, ...); Referenced from www.tutorialspoint.com
            sprintf(vector_memory_loc, "%03x", vector_memory_loc_int);
            printf("Command: %s, Vector Num: %d, Command Duration: %d, Memory loc: %s\n", curr_command, vector_num, curr_command_duration, vector_memory_loc);
        }
        else if (strncmp(curr_line, "CPU", 3) == 0)
        {
            curr_command_duration = 0;
            // take the line and split into curr_command and curr_command_duration
            //%[^,] reads everything until comma
            sscanf(curr_line, "%[^,], %d", curr_command, &curr_command_duration);
            printf("CPU cmd: %s, Duration: %d\n", curr_command, curr_command_duration);
        }
        else if (strncmp(curr_line, "FORK", 4) == 0)
        {
            sscanf(curr_line, "%[^,], %d", curr_command, &curr_command_duration);
            printf("FORK cmd: %s Duration: %d\n", curr_command, curr_command_duration);
            printf("---------------------------------------------\n");
        }
        else if (strncmp(curr_line, "EXEC", 4) == 0)
        {
            // Remove any newline characters from curr_line
            curr_line[strcspn(curr_line, "\n")] = '\0';

            // Parse the EXEC command
            // The format is: EXEC program_name, duration
            sscanf(curr_line, "%s %[^,], %d", curr_command, program_name, &curr_command_duration);

            // Remove any trailing commas or whitespace from program_name
            char *whitespace_coms = strchr(program_name, ','); // strchr looks for first occurrence of that character
            if (whitespace_coms != NULL)
            {
                *whitespace_coms = '\0';
            }

            // Trim any trailing whitespace
            while (isspace((unsigned char)program_name[strlen(program_name) - 1]))
            {
                program_name[strlen(program_name) - 1] = '\0';
            }

            printf("EXEC cmd: %s, Program Name: %s, Duration: %d\n", curr_command, program_name, curr_command_duration);
            printf("---------------------------------------------\n\n");
        }

        else
        {
            printf("Unknown command: %s\n", curr_line);
            continue;
        }

        // Execute the current ISR activity
        curr_time = isractivities(output_txt_file, curr_command, program_name, curr_command_duration, curr_time, vector_num, vector_memory_loc, vector_table, status_filename);
        printf("Current Time After ISR: %d\n", curr_time);
        printf("---------------------------------------------\n\n\n\n");
    }

    // Close the trace.txt file and output file
    fclose(trace_txt_file);
    fclose(output_txt_file);

    return 0;
}
// initialization func to load programs and their sizes into external files
void load_external_files(const char *external_filename, int trace_number)
{

    FILE *file = fopen(external_filename, "r");
    printf("EXTERNAL FILE BEING USED: %s\n", external_filename);
    printf("___________________________________________________________\n\n");

    if (file == NULL) // if no file or issue with file
    {
        printf("Error opening external files list: %s\n", external_filename);
        return;
    }

    char line[100];
    int file_index = 0;
    // getting the content from the file
    while (fgets(line, sizeof(line), file) != NULL)
    {
        char program_name[50];
        unsigned int program_size;

        // removing newline character
        line[strcspn(line, "\n")] = '\0';

        // parsing the line to get program name and size
        if (sscanf(line, "%[^,], %u", program_name, &program_size) == 2)
        {
            // storing the program name and size
            strcpy(external_files[file_index].program_name, program_name);
            external_files[file_index].program_size = program_size;

            // and then loading the actual program file
            char program_filename[100];
            sprintf(program_filename, "%s.txt", program_name);

            load_program_files(program_filename, file_index);

            file_index++; // next program entry
        }
    }

    fclose(file); // close file, good coding practice
}

// is triggered at the end of ISR
void routine_scheduler(FILE *output_txt_file, int *curr_time, int scheduler_time)
{

    fprintf(output_txt_file, "%d, %d, scheduler called\n", *curr_time, scheduler_time);
    *curr_time += scheduler_time;
}

// helper function to grab program size
unsigned int get_program_size(const char *program_name)
{
    for (int i = 0; i < MAX_NO_FILES; i++)
    {
        if (strlen(external_files[i].program_name) == 0)
            continue; // Skip empty entries

        if (strcmp(external_files[i].program_name, program_name) == 0)
        {
            return external_files[i].program_size;
        }
    }
    return 0; // not found
}

// helper function that communicates with the memory partition struct to help allocate memory to the programs based on their size
int find_suitable_partition(unsigned int program_size)
{
    int partition_index = -1;
    unsigned int min_size_diff = UINT_MAX;

    for (int i = 0; i < 6; i++)
    {
        if (strcmp(memory_partitions[i].partition_code, "unoccupied") == 0 &&
            memory_partitions[i].size_of_partition >= program_size)
        {
            unsigned int size_diff = memory_partitions[i].size_of_partition - program_size;
            if (size_diff < min_size_diff)
            {
                min_size_diff = size_diff;
                partition_index = i;
            }
        }
    }

    return partition_index; // returns -1 if no suitable partition found
}

// variables for the process mgt fork and exec
int next_pid = 1;
int process_count = 1;

// fork function: to implement creating child processes in the PCB
void fork_process(FILE *output_txt_file, int curr_time, int parent_pid)
{
    if (process_count < MAX_PROCESSES)
    {
        PCB *parent = &pcb_table[current_pcb_index];
        printf("\nParent pid at start: %d\n", parent_pid);
        PCB *child = &pcb_table[next_pid];
        printf("Next pid for child: %d\n", next_pid);

        child->pid = next_pid;
        child->parent_pid = parent_pid;
        strcpy(child->prog_name, parent->prog_name);
        printf("Prog name: %s\n", child->prog_name);
        child->remaining_cpu_time = parent->remaining_cpu_time;
        child->partition_no = parent->partition_no;
        strcpy(child->process_state, "Ready");
        strcpy(child->io_info, "None");

        process_count++;
        current_pid = child->pid;
        printf("Current pid in fork: %d\n", current_pid);
        next_pid++;
        printf("Next pid incremented: %d\n", next_pid);
        printf("---------------------------------------------\n\n");
    }
    else
    {
        fprintf(output_txt_file, "%d, ERROR, max process limit reached\n", curr_time);
    }
}

// function to implement exec and letting it reflect in the PCB
int exec_program(FILE *output_txt_file, int curr_time, int child_pid, const char *program_name, unsigned int program_size, int partition_index, char vector_table[25][7])
{
    PCB *child = &pcb_table[child_pid];

    // Mark partition as occupied
    strcpy(memory_partitions[partition_index].partition_code, program_name);

    // Update PCB with new info
    strcpy(child->prog_name, program_name);
    child->remaining_cpu_time = program_size;
    child->partition_no = memory_partitions[partition_index].partition_no;
    strcpy(child->process_state, "Running");

    return curr_time;
}

// function for saving/appending to the system_status txt file
void save_sys_status(int curr_time, const char *status_filename)
{
    FILE *status_file = fopen(status_filename, "a"); // Use "a" to append to the file

    fprintf(status_file, "!-----------------------------------------------------------!\n");
    fprintf(status_file, "Save Time: %d ms\n", curr_time);
    fprintf(status_file, "+------+----------------+-------------------+-------+\n");
    fprintf(status_file, "| PID  | Program Name   | Partition Number  | Size  |\n");
    fprintf(status_file, "+------+----------------+-------------------+-------+\n");

    for (int i = 0; i < next_pid; i++)
    {
        if (strcmp(pcb_table[i].process_state, "Terminated") != 0)
        {
            fprintf(status_file, "| %-4d | %-14s | %-17d | %-5u |\n",
                    pcb_table[i].pid,
                    pcb_table[i].prog_name ? pcb_table[i].prog_name : "init",
                    pcb_table[i].partition_no,
                    pcb_table[i].remaining_cpu_time);
        }
    }
    fprintf(status_file, "+------+----------------+-------------------+-------+\n");
    fprintf(status_file, "!-----------------------------------------------------------!\n");

    fclose(status_file);
    printf("\nSystem status saved at time %d\n", curr_time);
    printf("************************************************\n\n");
}

// function to help actually execute the commands within a program
int execute_program_commands(FILE *output_txt_file, int curr_time, int pid, char vector_table[25][7], const char *status_filename)
{
    PCB *process = &pcb_table[pid];

    // Get the program index
    int program_index = -1;
    for (int i = 0; i < MAX_NO_FILES; i++)
    {
        if (strcmp(external_files[i].program_name, process->prog_name) == 0)
        {
            program_index = i;
            break;
        }
    }
    if (program_index == -1)
    {
        fprintf(output_txt_file, "%d, ERROR: Program %s not found for execution\n", curr_time, process->prog_name);
        return curr_time;
    }

    // Execute each command associated with the program
    for (int cmd_index = 0; cmd_index < external_files[program_index].num_commands; cmd_index++)
    {
        printf("\nProcessing command %d: %s\n", cmd_index, external_files[program_index].commands[cmd_index]);
        char *command = external_files[program_index].commands[cmd_index];

        // Initialize variables
        char cmd_curr[100] = "";
        char cmd_program_name[100] = "";
        int cmd_duration = 0;
        int cmd_vector_num = 0;

        char vector_memory_loc[10] = "";

        // leading whitespace from command trimming
        char *trimmed_command = command;
        while (isspace((unsigned char)*trimmed_command)) // isspace from <ctype.h> helps here
            trimmed_command++;

        printf("Command line: '%s'\n", command);

        // get the command keyword
        if (sscanf(trimmed_command, "%[^ ,]", cmd_curr) != 1)
        {
            fprintf(output_txt_file, "%d, ERROR: Failed to parse command %s\n", curr_time, command);
            continue;
        }

        printf("Current command: '%s'\n", cmd_curr);

        // comparing the current cmd and parsing
        if ((strcmp(cmd_curr, "SYSCALL") == 0) || (strcmp(cmd_curr, "END_IO") == 0))
        {
            if (sscanf(trimmed_command, "%*s %d , %d", &cmd_vector_num, &cmd_duration) != 2)
            {
                fprintf(output_txt_file, "%d, ERROR: Invalid format in command %s\n", curr_time, command);
                continue;
            }
            vector_memory_loc_int = cmd_vector_num * 2;
            sprintf(vector_memory_loc, "%x", vector_memory_loc_int);
        }
        else if (strcmp(cmd_curr, "CPU") == 0)
        {
            if (sscanf(trimmed_command, "%*[^,], %d", &cmd_duration) != 1)
            {
                fprintf(output_txt_file, "%d, ERROR: Missing duration in command %s\n", curr_time, command);
                continue;
            }
        }
        else if (strcmp(cmd_curr, "FORK") == 0)
        {
            if (sscanf(trimmed_command, "%*[^,], %d", &cmd_duration) != 1)
            {
                fprintf(output_txt_file, "%d, ERROR: Missing duration in command %s\n", curr_time, command);
                continue;
            }
        }
        else if (strcmp(cmd_curr, "EXEC") == 0)
        {
            if (sscanf(trimmed_command, "%*s %[^,], %d", cmd_program_name, &cmd_duration) != 2)
            {
                fprintf(output_txt_file, "%d, ERROR: Invalid format in command %s\n", curr_time, command);
                continue;
            }
        }
        else
        {
            fprintf(output_txt_file, "%d, ERROR: Unknown command %s\n", curr_time, cmd_curr);
            continue;
        }

        // Call isractivities to process the command
        curr_time = isractivities(output_txt_file, cmd_curr, cmd_program_name, cmd_duration, curr_time,
                                  cmd_vector_num, vector_memory_loc, vector_table, status_filename);
    }

    // Switch back to parent process
    current_pid = process->parent_pid;

    return curr_time;
}

// handling isr, fork and exec, etc.
int isractivities(FILE *output_txt_file, char *curr_command, char *program_name, int curr_command_duration, int curr_time, int vector_num, char *vector_memory_loc, char vector_table[25][7], const char *status_filename)
{
    // Set the random time for the context switch between 1 and 3 ms
    int rand_context_switch_time = rand() % 3 + 1;

    // If the command is to the CPU (CPU)
    if (strncmp(curr_command, "CPU", 3) == 0)
    {
        // 1) Call CPU
        fprintf(output_txt_file, "%d, %d, CPU Execution\n", curr_time, curr_command_duration);
        // Update current / total time
        curr_time += curr_command_duration;
    }

    // If the command  is System Call (SYSCALL)
    else if (strcmp(curr_command, "SYSCALL") == 0)
    {

        // 1) Switch to kernel mode
        fprintf(output_txt_file, "%d, 1, Switch to kernel mode\n", curr_time);
        // Update current / total time
        curr_time += 1;

        // 2) Save context
        fprintf(output_txt_file, "%d, %d, Context saved\n", curr_time, rand_context_switch_time);
        // Update current / total time
        curr_time += rand_context_switch_time;

        // 3) Find vector location in memory
        fprintf(output_txt_file, "%d, 1, Find vector %d in memory position 0x000%s\n", curr_time, vector_num, vector_memory_loc);
        // Update current / total time
        curr_time += 1;

        // 4) Get ISR Address
        fprintf(output_txt_file, "%d, 1, load address %s into PC\n", curr_time, vector_table[vector_num]);
        // Update current / total time
        curr_time += 1;

        // Get random times for SYSCALL ISR activities
        int rand_syscall_duration = rand() % (curr_command_duration - 2) + 1;                                 // 1 to curr_command_duration - 2
        int rand_transfer_data_duration = rand() % (curr_command_duration - 1 - rand_syscall_duration) + 1;   // 1 to curr_command_duration - 1 - rand_syscall_duration
        int rand_error_check = (curr_command_duration - rand_syscall_duration - rand_transfer_data_duration); // remainder

        // 5) Run ISR
        fprintf(output_txt_file, "%d, %d, SYSCALL: run the ISR\n", curr_time, rand_syscall_duration);
        // Update current / total time with curr_command_duration
        curr_time += rand_syscall_duration;

        // 6) Transfer Data
        fprintf(output_txt_file, "%d, %d, Transfer Data\n", curr_time, rand_transfer_data_duration);
        // Update current / total time
        curr_time += rand_transfer_data_duration;

        // 7) Check for Errors
        fprintf(output_txt_file, "%d, %d, Check for Errors\n", curr_time, rand_error_check);
        // Update current / total time
        curr_time += rand_error_check;

        // 8) End ISR
        fprintf(output_txt_file, "%d, 1, IRET\n", curr_time);
        // Update current / total time
        curr_time += 1;
    }

    // If the command is end the interrupt (END_IO)
    else if (strcmp(curr_command, "END_IO") == 0)
    {
        // 1) Check Priority
        fprintf(output_txt_file, "%d, 1, Check Priority of Interrupt\n", curr_time);
        // Update current / total time
        curr_time += 1;

        // 2) Check if Masked
        fprintf(output_txt_file, "%d, 1, Check if Masked\n", curr_time);
        // Update current / total time
        curr_time += 1;

        // 3) Switch to kernel mode
        fprintf(output_txt_file, "%d, 1, Switch to kernel mode\n", curr_time);
        // Update current / total time
        curr_time += 1;

        // 4) Save context
        fprintf(output_txt_file, "%d, %d, Context saved\n", curr_time, rand_context_switch_time);
        // Update current / total time
        curr_time += rand_context_switch_time;

        // 5) Find vector location in memory
        fprintf(output_txt_file, "%d, 1, Find vector %d in memory position 0x000%s\n", curr_time, vector_num, vector_memory_loc);
        // Update current / total time
        curr_time += 1;

        // 6) Get ISR Address
        fprintf(output_txt_file, "%d, 1, load address %s into PC\n", curr_time, vector_table[vector_num]);
        // Update current / total time
        curr_time += 1;

        // 7) End Interrupt
        fprintf(output_txt_file, "%d, %d, End of I/O %d\n", curr_time, curr_command_duration, vector_num);
        // Update current / total time
        curr_time += curr_command_duration;

        // 8) IRET
        fprintf(output_txt_file, "%d, 1, IRET\n", curr_time);
        // Update current / total time
        curr_time += 1;
    }
    else if (strcmp(curr_command, "FORK") == 0)
    {
        // Simulate SYSCALL steps as per the example
        fprintf(output_txt_file, "%d, 1, switch to kernel mode\n", curr_time);
        curr_time += 1;

        fprintf(output_txt_file, "%d, %d, context saved\n", curr_time, rand_context_switch_time);
        curr_time += rand_context_switch_time;

        // Find vector 2 in memory position
        vector_num = 2;
        vector_memory_loc_int = vector_num * 2;
        sprintf(vector_memory_loc, "%x", vector_memory_loc_int);
        // Find vector location in memory
        fprintf(output_txt_file, "%d, 1, Find vector %d in memory position 0x000%s\n", curr_time, vector_num, vector_memory_loc);
        curr_time += 1;

        fprintf(output_txt_file, "%d, 1, load address %s into the PC\n", curr_time, vector_table[2]);
        curr_time += 1;

        // Get random times for exec activities
        int rand_copy = rand() % (curr_command_duration - 2) + 1; // 1 to curr_command_duration - 2
        int rand_scheduler_called = (curr_command_duration - rand_copy);

        // int fork_isr_time = curr_command_duration;
        fprintf(output_txt_file, "%d, %d, FORK: copy parent PCB to child PCB\n", curr_time, rand_copy);
        curr_time += rand_copy;

        // Call fork_process
        fork_process(output_txt_file, curr_time, current_pid);

        // At the end of ISR, call routine_scheduler
        routine_scheduler(output_txt_file, &curr_time, rand_scheduler_called);

        // Return from ISR
        fprintf(output_txt_file, "%d, 1, IRET\n", curr_time);
        curr_time += 1;

        save_sys_status(curr_time, status_filename);
    }

    else if (strcmp(curr_command, "EXEC") == 0)
    {

        // Getting program size
        program_size = get_program_size(program_name);
        if (program_size == 0)
        {
            fprintf(output_txt_file, "%d, ERROR: program %s not found\n", curr_time, program_name);
            return curr_time;
        }

        // Finding suitable partition
        partition_index = find_suitable_partition(program_size);
        if (partition_index == -1)
        {
            fprintf(output_txt_file, "%d, ERROR: No suitable partition found\n", curr_time);
            return curr_time;
        }

        partition_number = memory_partitions[partition_index].partition_no;
        partition_size = memory_partitions[partition_index].size_of_partition;

        // Simulate SYSCALL steps
        fprintf(output_txt_file, "%d, 1, Switch to kernel mode\n", curr_time);
        curr_time += 1;

        fprintf(output_txt_file, "%d, %d, Context saved\n", curr_time, rand_context_switch_time);
        curr_time += rand_context_switch_time;

        // Find vector 3 in memory position
        vector_num = 3;
        vector_memory_loc_int = vector_num * 2;
        sprintf(vector_memory_loc, "%x", vector_memory_loc_int);
        // Find vector location in memory
        fprintf(output_txt_file, "%d, 1, Find vector %d in memory position 0x000%s\n", curr_time, vector_num, vector_memory_loc);
        // Update current / total time
        curr_time += 1;

        // Load address into PC
        fprintf(output_txt_file, "%d, 1, load address %s into the PC\n", curr_time, vector_table[vector_num]);
        curr_time += 1;

        // Get random times for exec activities
        // doing it this way to make sure the EXEC activities' times all sum up to how much time EXEC cmd is supposed to take per the trace or program file
        int rand_load_duration = rand() % (curr_command_duration - 4) + 1;

        int rand_partition_found_duration = rand() % (curr_command_duration - 3 - rand_load_duration) + 1;

        int rand_mark = rand() % (curr_command_duration - 2 - rand_load_duration - rand_partition_found_duration) + 1;

        int rand_update = rand() % (curr_command_duration - 1 - rand_load_duration - rand_partition_found_duration - rand_mark) + 1;

        int rand_scheduler_called = (curr_command_duration - rand_load_duration - rand_partition_found_duration - rand_mark - rand_update);

        //  EXEC ISR: load program

        fprintf(output_txt_file, "%d, %d, EXEC: load %s of size %uMb\n", curr_time, rand_load_duration, program_name, program_size);
        curr_time += rand_load_duration;

        // Found partition

        fprintf(output_txt_file, "%d, %d, found partition %d with %uMb of space\n", curr_time, rand_partition_found_duration, partition_number, partition_size);
        curr_time += rand_partition_found_duration;

        // Partition marked as occupied

        fprintf(output_txt_file, "%d, %d, partition %d marked as occupied\n", curr_time, rand_mark, partition_number);
        curr_time += rand_mark;

        // Updating PCB

        fprintf(output_txt_file, "%d, %d, updating PCB with new information\n", curr_time, rand_update);
        curr_time += rand_update;

        // Call exec_program function and update curr_time
        curr_time = exec_program(output_txt_file, curr_time, current_pid, program_name, program_size, partition_index, vector_table);

        // At the end of ISR, call routine_scheduler
        routine_scheduler(output_txt_file, &curr_time, rand_scheduler_called);

        // Return from ISR
        fprintf(output_txt_file, "%d, 1, IRET\n", curr_time);
        curr_time += 1;

        save_sys_status(curr_time, status_filename);

        curr_time = execute_program_commands(output_txt_file, curr_time, current_pid, vector_table, status_filename);
    }

    else
    {
        fprintf(output_txt_file, "%d, ERROR: Unknown command %s\n", curr_time, curr_command);
    }

    return curr_time;
}
