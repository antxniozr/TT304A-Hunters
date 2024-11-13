#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <limits.h> // Inclui INT_MAX
#include <string.h>

#define MAX_THREADS 8 // Num máximo de threads

// Estrutura para armazenar dados da thread
typedef struct 
{
    int thread_id;        
    int *data;            
    int data_size;        
    double execution_time;   
    char *filename;          
} ThreadData;

// Função para mesclar duas metades ordenadas de um array
void merge(int *data, int left, int middle, int right) 
{
    int i, j, k;
    int n1 = middle - left + 1; // Tamanho da primeira 
    int n2 = right - middle;    // Tamanho da segunda metade

    // Aloca vetores temporários para armazenar as metades
    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));

    // Copia os dados para os vetores temporários L e R, esquerda e direita
    for (i = 0; i < n1; i++)
    {
       L[i] = data[left + i];
    }
    for (j = 0; j < n2; j++)
    {
        R[j] = data[middle + 1 + j];
    }

    // Mescla os vetores temporários de volta em data
    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) 
    {
        if (L[i] <= R[j]) 
        {
            data[k] = L[i];
            i++;
        } 
        else 
        {
            data[k] = R[j];
            j++;
        }
        k++;
    }

    // Copia os elementos restantes de L, se houver
    while (i < n1) 
    {
        data[k] = L[i];
        i++;
        k++;
    }
 
    // Copia os elementos restantes de R, se houver
    while (j < n2) 
    {
        data[k] = R[j];
        j++;
        k++;
    }
 
    // Libera a memória dos vetores temporários
    free(L);
    free(R);
}

// Função principal do mergesort
void merge_sort(int *data, int left, int right) 
{
    if (left < right) 
    {
        int middle = left + (right - left) / 2;// Determina a posição do valor central do vetor
        merge_sort(data, left, middle);        // Organiza a primeira metade
        merge_sort(data, middle + 1, right);   // Organiza a segunda metade
        merge(data, left, middle, right);      // Junta as duas metades e organiza
    }
} 

// Função executada por cada thread para processar um arquivo
void *process_file(void *arg) 
{
    ThreadData *t_data = (ThreadData *)arg;
    struct timespec start, end; //Variáveis para marcar o começo(start) e o fim(end) de um thread
 
    // Registra o inicio da thread
    clock_gettime(CLOCK_MONOTONIC, &start);
 
    // Abre o arquivo para leitura
    FILE *file = fopen(t_data->filename, "r");
    if (file == NULL) 
    {
        fprintf(stderr, "Erro ao abrir o arquivo %s\n", t_data->filename);
        pthread_exit(NULL);
    }
 
    // Lê os valores do arquivo e armazena no vetor
    int value;
    int size = 0;
    while (fscanf(file, "%d", &value) != EOF) 
    {
        t_data->data[size++] = value;
    }
    fclose(file); // Fecha o arquivo após a leitura
 
    // Ordena os dados usando merge_sort
    merge_sort(t_data->data, 0, size - 1);
 
    // Registra o tempo de término da thread e calcula o tempo de execução
    clock_gettime(CLOCK_MONOTONIC, &end);
    t_data->execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9; // Calcula o tempo de execução e o transforma em segundos
    t_data->data_size = size; // Armazena o tamanho dos dados processados
    pthread_exit(NULL);
}

// Mesclar os dados de todas as threads em um único array ordenado
int* final_merge(ThreadData thread_data[], int num_threads, int total_size) 
{
    int *merged_data = (int *)malloc(total_size * sizeof(int)); // Array final em ordem
    int *indices = (int *)calloc(num_threads, sizeof(int));     // Array de índices para cada thread
 
    // Mescla os dados de cada thread no array final
    for (int i = 0; i < total_size; i++) 
    {
        int min_val = INT_MAX;
        int min_idx = -1;
 
        // Encontra o menor valor disponível entre as threads
        for (int j = 0; j < num_threads; j++) 
        {
            if (indices[j] < thread_data[j].data_size && thread_data[j].data[indices[j]] < min_val) 
            {
                min_val = thread_data[j].data[indices[j]]; //Atualiza min_val para o menor valor encontrado
                min_idx = j; // Atualiza min_idx para o indice da thread que tem o menor valor
            }
        }
 
        merged_data[i] = min_val;   // Adiciona o menor valor ao array final
        indices[min_idx]++;         // Incrementa o índice da thread que forneceu o menor valor
    }
 
    free(indices); // Libera a memória dos índices temporários
    return merged_data;
}

int main(int argc, char *argv[]) 
{
    if (argc < 5) // Verifica se o numero de argumentos é menor que 5, caso sim quebra o programa
    {
        fprintf(stderr, "Uso: %s <num_threads> <arq1> <arq2> ... -o <saida>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
 
    // Cria o número de threads da linha de comando
    int num_threads = atoi(argv[1]);
    if (num_threads > MAX_THREADS) 
    {
        num_threads = MAX_THREADS;
    }
 
    // Arrays para armazenar os arquivos de entrada e o arquivo de saída
    char *input_files[argc - 3];
    int input_count = 0;
    char *output_file = NULL;
 
    //Começa o laço no número de threads
    for (int i = 2; i < argc; i++) 
    {
        if (strcmp(argv[i], "-o") == 0) // Verifica se o argumento atual é o -o
        {
            if (i + 1 < argc) // Verificar se existe um argumento depois do -o
            {
                output_file = argv[i + 1]; //Define o proximo argumento como o arquivo de saída
                break;
            } 
            else 
            {
                fprintf(stderr, "Erro: Arquivo de saída não especificado.\n"); // Exibe um erro se -o for ultimo argumento e sai do programa
                exit(EXIT_FAILURE);
            }
        } 
        else 
        {
            input_files[input_count++] = argv[i]; // Armazena os arquivos de entrada enquanto não encontrar o -o
        }
    }
 
    if (output_file == NULL) 
    {
        fprintf(stderr, "Erro: O argumento '-o' e o arquivo de saída são obrigatórios.\n");
        exit(EXIT_FAILURE);
    }
 
    // Declaração das threads
    ThreadData thread_data[num_threads];
    pthread_t threads[num_threads];
    struct timespec total_start, total_end;
    clock_gettime(CLOCK_MONOTONIC, &total_start);
 
    int total_size = 0;
    for (int i = 0; i < num_threads; i++) 
    {
        thread_data[i].thread_id = i;
        thread_data[i].filename = input_files[i % input_count];   // Distribui os arquivos de entrada entre as threads
        thread_data[i].data = (int *)malloc(10000 * sizeof(int)); // Aloca até 10000 inteiros para cada arquivo
 
        pthread_create(&threads[i], NULL, process_file, (void *)&thread_data[i]); //  Cria uma nova thread
    }
 
    for (int i = 0; i < num_threads; i++) 
    {
        pthread_join(threads[i], NULL);
        total_size += thread_data[i].data_size; // Soma o tamanho dos dados de cada thread
        printf("Tempo de execução do Thread %d: %f segundos.\n", i, thread_data[i].execution_time); // Exibe o tempo de exibição de cada thread
    }
 
    // Calcula o tempo total de execução
    clock_gettime(CLOCK_MONOTONIC, &total_end);
    double total_time = (total_end.tv_sec - total_start.tv_sec) + (total_end.tv_nsec - total_start.tv_nsec) / 1e9; // Calcula o tempo total e converte para segundos
    printf("Tempo total de execução: %f segundos.\n", total_time); // Exibe o tempo total de execução do programa
 
    // Mesclagem final dos dados ordenados de todas as threads
    int *merged_data = final_merge(thread_data, num_threads, total_size);
 
    // Escreve os dados ordenados no arquivo de saída
    FILE *output = fopen(output_file, "w");
    if (output == NULL) 
    {
        perror("Erro ao abrir o arquivo de saída");
        free(merged_data);
        return 1;
    }
 
    for (int i = 0; i < total_size; i++) 
    {
       fprintf(output, "%d\n", merged_data[i]);
    }
 
    fclose(output);
    free(merged_data);
 
    // Libera a memória alocada para os dados de cada thread
    for (int i = 0; i < num_threads; i++) 
    {
        free(thread_data[i].data);
    }
 
    return 0;
}

