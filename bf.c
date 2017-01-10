#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define ARRAY_MAX 30000
#define ARRAY_START 512
#define CMD_SIZE 512
#define STACK_START 64

struct stack
{
    size_t *array;
    size_t size;
    size_t top;
    size_t capacity;
}

void pop(struct stack *s);
void push(struct stack *s, size_t val);
size_t top(struct stack *s);
int is_empty(struct stack *s);

int grow_array(void** ptr, size_t* array_size, size_t array_max,
               size_t ptr_size);

int run_bf(FILE *in, FILE *out, size_t bytes_read, int *bf_array,
           size_t *array_size, size_t *offset, size_t array_max, char *commands,
           size_t instruction_ptr);

int main(int argc, char** argv)
{
    struct timeval start;              /*program start time*/
    struct timeval end;                /*program end time*/
    FILE *source = stdin;              /*bf source code stream*/
    FILE *in = stdin;                  /*input to bf script*/
    FILE *out = stdout;                /*bf output*/
    size_t bytes_read = 0;             /*bytes read in most recent source read*/
    int *bf_array = NULL;              /*bf stack*/
    size_t array_size = ARRAY_START/2; /*size of the bf_array*/
    size_t offset = 0;                 /*offset in to the bf_array*/
    size_t array_max = ARRAY_MAX;      /*maximum size of bf_array*/
    char *commands;                    /*the bf commands i.e. script*/
    size_t command_size = 0;           /*used size of command buffer*/
    size_t command_max = CMD_SIZE/2;   /*allocated size of command buffer*/
    size_t instruction_ptr = 0;        /*offset into commands*/
    int quiet_flg = 0;                 /*suppress end of job message*/

    gettimeofday(&start, NULL);

/*------------------------------PARSE ARGUMENTS-------------------------------*/

    int arg;
    for(arg = 1; arg < argc; ++arg)
    {
        fprintf(out, "arg %d: %s\n", arg, argv[arg]);
        if(argv[arg][0] == '-')
        {
            if(argv[arg][1] == 'f')
            {
                fprintf(stderr, "open source file: %s\n", argv[++arg]);
                source = fopen(argv[arg], "r");
            }
            if(argv[arg][1] == 'i')
            {
                fprintf(stderr, "open input file: %s\n", argv[++arg]);
                in = fopen(argv[arg], "r");
            }
            if(argv[arg][1] == 'o')
            {
                fprintf(stderr,  "open output file: %s\n", argv[++arg]);
                out = fopen(argv[arg], "w");
            }
            if(argv[arg][1] == 'm')
            {
                array_max = atoll(argv[++arg]);
            }
            if(argv[arg][1] == 'q')
            {
                quiet_flg = 1;
            }
        }
        else /*argument with no - is considered a source file and end of args*/
        {
            fprintf(stderr, "open file: %s\n", argv[arg]);
            source = fopen(argv[arg], "r");
            break;
        }
    }

/*--------------------------------RUN PROGRAM---------------------------------*/

    grow_array((void**)&commands, &command_max, 0, sizeof(char));

    grow_array((void**)&bf_array, &array_size, array_max, sizeof(int));

    while((command_max - command_size) < CMD_SIZE)
    {
        grow_array((void**)&commands, &command_max, 0, sizeof(char));
    }
    
    bytes_read = fread(commands+command_size, 1, CMD_SIZE, source);
    command_size += bytes_read;
    
    fprintf(out, "commands:\n");

    while(bytes_read > 0)
    {
        while(instruction_ptr < command_size)
        {
            printf("offset: %zd -- command: %.1s\n", instruction_ptr,
                   commands+instruction_ptr);
            fprintf(out, "%.1s", commands + instruction_ptr++);
        }
        
        if(!feof(source))
        {
            while((command_max - command_size) < CMD_SIZE)
            {
                grow_array((void**)&commands, &command_max, 0, sizeof(char));
            }
            bytes_read = fread(commands+command_size, 1, CMD_SIZE, source);
        }
        else
        {
            bytes_read = 0;
        }
        command_size += bytes_read;
    }

/*----------------------------------CLEANUP-----------------------------------*/

    if(source != stdin)
    {
        fclose(source);
    }
    if(in != stdin)
    {
        fclose(in);
    }
    if(out != stdin)
    {
        fclose(out);
    }

    free(bf_array);
    free(commands);

    gettimeofday(&end, NULL);

    if(quiet_flg == 0)
    {
        fprintf(stderr, "source bytes: %zd\nexecution time: %f s\n",
                command_size, ((end.tv_sec*1000000.0+end.tv_usec)
                -(start.tv_sec*1000000.0+start.tv_usec))/1000000.0);
    }

    return 0;
}

//fix this!!!
int run_bf(FILE *in, FILE *out, size_t bytes_read, int *bf_array,
           size_t *array_size, size_t *offset, size_t array_max, char *commands,
           size_t *instruction_ptr)
{
    while(instruction_ptr < command_size)
    {
        if(*instruction_ptr == '<')
        {
            --*offset;
        }
        if(*instruction_ptr == '>')
        {
            ++*offset;
        }
        if(*instruction_ptr == '+')
        {
            ++(bf_array[*offset]);
        }
        if(*instruction_ptr == '-')
        {
            --(bf_array[*offset]);
        }
        if(*instruction_ptr == '[')
        {
            if(bf_array[*offset] == 0)
            {
                size_t brace_count = 1;
                while(brace_count > 0)
                {
                    if(bf_array[*instruction_ptr] == '[') ++brace_count;
                    else if(bf_array[*instruction_ptr] == ']') --brace_count;
                    ++instruction_ptr;
                }
            }
        }
        if(*instruction_ptr == ']')
        {
            /*make this use the stack eventually*/
            if(bf_array[*offset] != 0)
            {
                size_t brace_count = 1;
                while(brace_count > 0)
                {
                    if(bf_array[*instruction_ptr] == ']') ++brace_count;
                    else if(bf_array[*instruction_ptr] == '[') --brace_count;
                    --instruction_ptr;
                }
            }
        }
        if(*instruction_ptr == '.')
        {
            fprintf(out, bf_array[*offset]);
        }
        if(*instruction_ptr == ',')
        {

        }
    }

    return 0;
}

int grow_array(void** ptr, size_t* array_size, size_t array_max,
               size_t ptr_size)
{
    fprintf(stderr, "grow array:\n\tptr: %p\n\tarray size: %zd\n\tarray max: %zd\n\tptr size: %zd\n", ptr, *array_size, array_max, ptr_size);
    void *new_ptr;
    int ret = 0;

    if(array_max > 0 && *array_size == array_max)
    {
        ret = 1;
    }
    else
    {
        *array_size += *array_size; //double it
        if(array_max > 0 && *array_size > array_max)
        {
            *array_size = array_max;
        }
        new_ptr = realloc(*ptr, *array_size * ptr_size);
        if(new_ptr == NULL)
        {
            ret = 2; //realloc failed (probably out of memory)
        }
        else
        {
            *ptr = new_ptr;
        }
    }
    fprintf(stderr, "\tret: %i\n", ret);
    return ret;
}

/*
struct stack
{
    size_t *array;
    size_t size;
    size_t top;
    size_t capacity;
}
*/

void pop(struct stack *s)
{
    if(s->top > 0)
    {
        --top;
    }
    if(s->size > 0)
    {
        --(s->size);
    }
}
void push(struct stack *s, size_t val)
{
    ++(s->size);
    if(s->size == s->capacity)
    {
        //grow array
    }
    ++(s->top);
    s->array[s->top] = val;
}

size_t top(struct stack *s)
{
    return s->array[top];
}

int is_empty(struct stack *s)
{
    return (s->size == 0);
}

