#include "processor.h"
#include "stack.h"

//===================================================================================================

#define PRINT_BEGIN() fprintf (log_file, "begin: %s.\n", __PRETTY_FUNCTION__)

#define PRINT_END() fprintf (log_file, "success end: %s\n", __PRETTY_FUNCTION__)

static FILE* log_file = stderr;

static const size_t num_of_double = 25;

enum Proc_error proc_set_log_file (FILE* file)
{
    PRINT_BEGIN();
    if (file == NULL)
        return PROC_NULL_PTR_LOG;
    log_file = file;
    PRINT_END();
    return PROC_NO_ERROR;
}

enum Proc_error check_argc (int argc)
{
    PRINT_BEGIN();
    if (argc > 2)
        return PROC_ERROR_ARGC;
    PRINT_END();
    return PROC_NO_ERROR;
}

enum Proc_error open_file_and_fill_stat (const char *NAME, FILE** file, struct stat* statbuf)
{
    PRINT_BEGIN();
    *file = fopen (NAME, "rb");
    if (*file == NULL)
        return PROC_ERROR_FOPEN;
    if (stat (NAME, statbuf))
        return PROC_ERROR_STAT;
    printf ("%zu\n", statbuf->st_size);
    PRINT_END();
    return PROC_NO_ERROR;
}

void proc_print_error (enum Proc_error error)
{
    PRINT_BEGIN();
    fprintf (log_file, "%s\n", proc_get_error (error));
    PRINT_END();
}

enum Proc_error calculations (struct Stack* stk, char* buffer, struct stat statbuf)
{
    PRINT_BEGIN();
    enum StkError error = STK_NO_ERROR;
    stk_element reg[4] = {};
    stk_element a = 0;
    stk_element b = 0;
    for (size_t position = 0; position < statbuf.st_size; position++)
    {
        printf ("buffer[position] = %d\n", buffer[position]);
        printf ("Крайнее число стэка: %g\n", *((char*) stk->data + stk->size));
        switch ((char) buffer[position])
        {
            case IN:
                printf ("Я в IN!!!\n");
                printf ("Enter your double in stack.\n");
                scanf ("%lf", &a);
                printf ("Толкаю в стэк %lf\n", a);
                error = stack_push (stk, a);
                if (error)
                    return PROC_STACK_PUSH_ERROR;
                break;
            case PUSH + NUM:
                fprintf (log_file, "case PUSH + NUM\n");
                printf ("%d\n", buffer[position]);
                error = stack_push (stk, *(double*)(buffer + position + sizeof (char)));
                printf ("%lf\n", *((char*) stk->data + position));
                position += sizeof (double);
                if (error)
                    return PROC_STACK_PUSH_ERROR;
                break;
            case PUSH + REG:
                fprintf (log_file, "case PUSH + REG\n");
                printf ("Номер регистра ");
                error = stack_push (stk, reg[*(char*)(buffer + position + sizeof (char))]);
                position += sizeof (char);
                if (error)
                    return PROC_STACK_PUSH_ERROR;
                break;
            case POP + REG:
                fprintf (log_file, "case POP + REG\n");
                error = stack_pop (stk, &a);
                printf ("%d\n", *(char*)(buffer + position));
                printf ("Номер регистра = %d\n", *(char*)(buffer + position + sizeof (char)));
                reg[*(char*)(buffer + position + sizeof (char))] = a;
                position += sizeof (char);
                if (error)
                    return PROC_STACK_PUSH_ERROR;
                break;
            case ADD:
                fprintf (log_file, "case ADD\n");
                error = stack_pop (stk, &a);
                if (error)
                    return PROC_STACK_POP_ERROR;
                error = stack_pop (stk, &b);
                if (error)
                    return PROC_STACK_POP_ERROR;
                error = stack_push (stk, a + b);
                if (error)
                    return PROC_STACK_PUSH_ERROR;
                break;
            case SUB:
                fprintf (log_file, "case SUB\n");
                error = stack_pop (stk, &a);
                if (error)
                    return PROC_STACK_POP_ERROR;
                error = stack_pop (stk, &b);
                if (error)
                    return PROC_STACK_POP_ERROR;
                error = stack_push (stk, b - a);
                if (error)
                    return PROC_STACK_PUSH_ERROR;
                break;
            case MUL:
                fprintf (log_file, "case MUL\n");
                error = stack_pop (stk, &a);
                printf ("a = %d\n", a);
                printf ("error = %d\n", error);
                if (error)
                    return PROC_STACK_POP_ERROR;
                error = stack_pop (stk, &b);
                printf ("b = %d\n", b);
                if (error)
                    return PROC_STACK_POP_ERROR;
                error = stack_push (stk, a * b);
                if (error)
                    return PROC_STACK_PUSH_ERROR;
                break;
            case DIV:
                fprintf (log_file, "case DIV\n");
                error = stack_pop (stk, &a);
                if (error)
                    return PROC_STACK_POP_ERROR;
                error = stack_pop (stk, &b);
                if (error)
                    return PROC_STACK_POP_ERROR;
                error = stack_push (stk, b / a);
                if (error)
                    return PROC_STACK_PUSH_ERROR;
                break;
            case JMP + NUM:
                fprintf (log_file, "case JMP\n");
                printf ("position ==== %d\n", position);
                position = *(double*)(buffer + position + sizeof (char));
                position--;
                printf ("position == %d\n", position);
                break;
            case OUT:
                fprintf (log_file, "case OUT\n");
                error = stack_pop (stk, &a);
                if (error)
                    return PROC_STACK_POP_ERROR;
                fprintf (log_file, "Результат: %lg\n", a);
                break;
            case HLT:
                return PROC_NO_ERROR;
            default:
                printf ("<%d>", buffer[position]);
                fprintf (log_file, "Зашёл в default\n");
                return PROC_ERROR_CMDS;
        }
    }
    PRINT_END();
    return PROC_NO_ERROR;
}

/*enum Proc_error processing_file (const char* NAME, char** buffer, FILE** code_file, double** array, struct stat* statbuf)
{
    PRINT_BEGIN();
    enum Proc_error proc_error = PROC_NO_ERROR;
    proc_error = open_file_and_fill_stat (NAME, code_file, statbuf);
    if (proc_error != PROC_NO_ERROR)
        return proc_error;
    char* temp = (char*) calloc (statbuf->st_size, sizeof (char));
    if (temp == NULL)
        return PROC_CALLOC_FAIL;
    *buffer = temp;
    printf ("%d\n", fread (*buffer, sizeof (char), statbuf->st_size, *code_file));
    *array = (double*) calloc (num_of_double, sizeof (double));
    proc_error = char_to_double (*buffer, *array);
    if (proc_error != PROC_NO_ERROR)
        return proc_error;
    PRINT_END();
    return PROC_NO_ERROR;
}*/

enum Proc_error processing_file (const char* NAME, char** buffer, FILE** code_file, struct stat* statbuf)
{
    PRINT_BEGIN();
    enum Proc_error proc_error = PROC_NO_ERROR;
    proc_error = open_file_and_fill_stat (NAME, code_file, statbuf);
    if (proc_error != PROC_NO_ERROR)
        return proc_error;
    char* temp = (char*) calloc (statbuf->st_size, sizeof (char));
    if (temp == NULL)
        return PROC_CALLOC_FAIL;
    *buffer = temp;
    printf ("%d\n", fread (*buffer, sizeof (char), statbuf->st_size, *code_file));
    PRINT_END();
    return PROC_NO_ERROR;
}

/*enum Proc_error char_to_double (char* buffer, double* array)
{
    PRINT_BEGIN();
    char* ptr_to_remains = NULL;
    size_t pass = 0;
    size_t num_if_overflow = num_of_double;
    while (*buffer != '\0')
    {
        if (pass >= num_of_double)
        {
            array = (double*) realloc (array, num_if_overflow * 2);
            num_if_overflow *= 2;
        }
        array[pass] = strtod (buffer, &ptr_to_remains);
        printf ("%.2lf\n\n", array[pass]);
        buffer = ptr_to_remains;
        pass++;
        while (isspace(*buffer))
            buffer++;
    }
    return PROC_NO_ERROR;
}*/



const char* proc_get_error (enum Proc_error error)
{
    switch (error)
    {
        case PROC_NO_ERROR:
            return "Proc: Ошибок в работе функций не выявлено.";
        case PROC_ERROR_FOPEN:
            return "Proc: Ошибка в работе функции fopen.";
        case PROC_ERROR_CMDS:
            return "Proc: Ошибка распознования команды.";
        case PROC_ERROR_STAT:
            return "Proc: Ошибка выполнения функции stat.";
        case PROC_ERROR_FSEEK:
            return "Proc: Ошибка выполнения функции fseek.";
        case PROC_ERROR_ARGC:
            return "Proc: Введены некорректные аргументы.";
        case PROC_NULL_PTR_LOG:
            return "Proc: Нулевой указатель на log.";
        case PROC_NULL_PTR_FILE:
            return "Proc: Нулевой указатель на файл.";
        case PROC_CALLOC_FAIL:
            return "Proc: Ошибка функции calloc.";
        case PROC_STACK_POP_ERROR:
            return "Proc: Ошибка функции pop.";
        case PROC_STACK_PUSH_ERROR:
            return "Proc: Ошибка функции push.";
        default:
            return "Stack: Куда делся мой enum ошибок?";
    }
}
