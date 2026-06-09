#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <unistd.h>

// Cores ANSI para deixar o terminal estilizado e moderno (premium)
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_BOLD          "\x1b[1m"

/**
 * Exibe a barra de progresso de todos os núcleos concorrentes.
 */
void print_progress_bars(volatile long long *progress, volatile long long *total, int num_threads, int first_time) {
    if (!first_time) {
        // Move o cursor do terminal para cima 'num_threads' linhas para redesenhar
        printf("\033[%dA", num_threads);
    }
    for (int i = 0; i < num_threads; i++) {
        double pct = 0.0;
        long long t = total[i];
        long long p = progress[i];
        if (t > 0) {
            pct = (double)p / t * 100.0;
        }
        if (pct > 100.0) pct = 100.0;
        if (pct < 0.0) pct = 0.0;
        
        int bar_width = 30;
        int filled = (int)(pct / 100.0 * bar_width);
        
        // Intercala cores para facilitar visualização de cada núcleo
        const char* colors[] = {
            ANSI_COLOR_GREEN,
            ANSI_COLOR_CYAN,
            ANSI_COLOR_BLUE,
            ANSI_COLOR_MAGENTA,
            ANSI_COLOR_YELLOW
        };
        const char* color = colors[i % 5];
        
        printf(ANSI_COLOR_BLUE "Núcleo %2d: [" ANSI_COLOR_RESET "%s", i, color);
        for (int j = 0; j < bar_width; j++) {
            if (j < filled) {
                printf("█");
            } else {
                printf(" ");
            }
        }
        printf(ANSI_COLOR_BLUE "]" ANSI_COLOR_RESET " " ANSI_BOLD "%5.1f%%\n" ANSI_COLOR_RESET, pct);
    }
    fflush(stdout);
}

/**
 * Realiza o cálculo de PI de forma sequencial com barra de progresso.
 */
double calcular_pi_sequencial(long long num_steps) {
    double step = 1.0 / (double)num_steps;
    double sum = 0.0;
    long long step_interval = num_steps / 100;
    if (step_interval == 0) step_interval = 1;
    
    // Ocultar cursor do terminal para evitar piscadas
    printf("\033[?25l");
    
    int bar_width = 30;
    printf("Sequencial: [                              ]   0.0%%\n");
    fflush(stdout);
    
    for (long long i = 0; i < num_steps; i++) {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
        if (i % step_interval == 0 || i == num_steps - 1) {
            double pct = (double)i / num_steps * 100.0;
            if (i == num_steps - 1) pct = 100.0;
            
            int filled = (int)(pct / 100.0 * bar_width);
            printf("\033[1A"); // Sobe 1 linha
            printf("\r" ANSI_COLOR_BLUE "Sequencial: [" ANSI_COLOR_YELLOW);
            for (int j = 0; j < bar_width; j++) {
                if (j < filled) printf("█");
                else printf(" ");
            }
            printf(ANSI_COLOR_BLUE "] " ANSI_COLOR_YELLOW "%5.1f%%\n" ANSI_COLOR_RESET, pct);
            fflush(stdout);
        }
    }
    
    // Restaurar cursor do terminal
    printf("\033[?25h");
    return step * sum;
}

/**
 * Realiza o cálculo de PI em paralelo, gerenciando uma thread monitora separada.
 */
double calcular_pi_paralelo(long long num_steps, int num_threads) {
    double step = 1.0 / (double)num_steps;
    double sum = 0.0;
    
    // Alocação de memória dinâmica para acompanhar o progresso de cada thread (volatile para forçar gravação em RAM)
    volatile long long *thread_progress = (volatile long long *)calloc(num_threads, sizeof(volatile long long));
    volatile long long *thread_total = (volatile long long *)calloc(num_threads, sizeof(volatile long long));
    volatile int workers_active = num_threads;
    
    // Ocultar cursor
    printf("\033[?25l");
    
    // Reserva o espaço inicial no terminal (evita que a tela suba desorganizadamente)
    for (int i = 0; i < num_threads; i++) {
        printf("\n");
    }
    fflush(stdout);
    
    // Iniciamos num_threads + 1 threads (num_threads workers + 1 thread monitora)
    #pragma omp parallel num_threads(num_threads + 1)
    {
        int tid = omp_get_thread_num();
        if (tid == num_threads) {
            // Thread de monitoramento (Lógica da interface de progresso)
            usleep(10000); // Dá um tempo curto para inicialização dos workers
            
            while (1) {
                int active;
                #pragma omp atomic read
                active = workers_active;
                
                print_progress_bars(thread_progress, thread_total, num_threads, 0);
                
                if (active == 0) {
                    break;
                }
                usleep(30000); // Atualiza o terminal a cada ~30ms (aproximadamente 33 FPS)
            }
            // Força a exibição final de todos a 100%
            print_progress_bars(thread_progress, thread_total, num_threads, 0);
        } else {
            // Worker Thread (Lógica matemática do cálculo)
            long long start = tid * (num_steps / num_threads);
            long long end = (tid == num_threads - 1) ? num_steps : (tid + 1) * (num_steps / num_threads);
            thread_total[tid] = end - start;
            
            double local_sum = 0.0;
            long long step_interval = (end - start) / 100;
            if (step_interval == 0) step_interval = 1;
            
            for (long long i = start; i < end; i++) {
                double x = (i + 0.5) * step;
                local_sum += 4.0 / (1.0 + x * x);
                if ((i - start) % step_interval == 0) {
                    thread_progress[tid] = i - start;
                }
            }
            thread_progress[tid] = end - start; // Concluiu
            
            #pragma omp atomic
            sum += local_sum;
            
            #pragma omp atomic
            workers_active--;
        }
    }
    
    // Restaurar cursor
    printf("\033[?25h");
    
    free((void *)thread_progress);
    free((void *)thread_total);
    
    return step * sum;
}

int main() {
    long long num_steps = 1000000000LL; // Padrão: 1 bilhão
    int num_threads = 8;               // Padrão: 8 núcleos
    char buffer[100];
    
    // Cabeçalho de Boas-vindas
    printf(ANSI_COLOR_CYAN ANSI_BOLD "\n==================================================\n");
    printf("     CÁLCULO PARALELO DE PI COM OPENMP            \n");
    printf("==================================================\n" ANSI_COLOR_RESET);
    
    // Detecta a quantidade total de threads lógicas disponíveis
    int max_cores = omp_get_max_threads();
    printf("Núcleos (Threads) lógicos disponíveis no sistema: " ANSI_BOLD "%d\n\n" ANSI_COLOR_RESET, max_cores);
    
    // Solicitar iterações com valor padrão
    printf("Informe o número de iterações [" ANSI_BOLD "1.000.000.000" ANSI_COLOR_RESET " - Enter para padrão]: ");
    fflush(stdout);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (buffer[0] != '\n' && buffer[0] != '\0') {
            long long val;
            if (sscanf(buffer, "%lld", &val) == 1 && val > 0) {
                num_steps = val;
            }
        }
    }
    
    // Solicitar núcleos com valor padrão
    printf("Informe o número de núcleos (threads) [" ANSI_BOLD "8" ANSI_COLOR_RESET " - Enter para padrão]: ");
    fflush(stdout);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (buffer[0] != '\n' && buffer[0] != '\0') {
            int val;
            if (sscanf(buffer, "%d", &val) == 1 && val > 0) {
                num_threads = val;
            }
        }
    }
    
    // 1. Cálculo Sequencial
    printf(ANSI_COLOR_YELLOW "\n[1/2] Iniciando cálculo SEQUENCIAL...\n" ANSI_COLOR_RESET);
    double start_seq = omp_get_wtime();
    double pi_seq = calcular_pi_sequencial(num_steps);
    double end_seq = omp_get_wtime();
    double time_seq = end_seq - start_seq;
    printf(ANSI_COLOR_GREEN "✔ Sequencial concluído em %.6f segundos.\n" ANSI_COLOR_RESET, time_seq);
    
    // 2. Cálculo Paralelo
    printf(ANSI_COLOR_YELLOW "\n[2/2] Iniciando cálculo PARALELO utilizando %d núcleos...\n" ANSI_COLOR_RESET, num_threads);
    double start_par = omp_get_wtime();
    double pi_par = calcular_pi_paralelo(num_steps, num_threads);
    double end_par = omp_get_wtime();
    double time_par = end_par - start_par;
    printf(ANSI_COLOR_GREEN "✔ Paralelo concluído em %.6f segundos.\n" ANSI_COLOR_RESET, time_par);
    
    // Estatísticas adicionais
    double speedup = time_seq / time_par;
    double efficiency = (speedup / num_threads) * 100.0;
    double error_seq = fabs(pi_seq - M_PI);
    double error_par = fabs(pi_par - M_PI);
    
    // Painel de Resultados
    printf(ANSI_COLOR_CYAN ANSI_BOLD "\n==================================================\n");
    printf("                TABELA DE RESULTADOS              \n");
    printf("==================================================\n" ANSI_COLOR_RESET);
    
    printf(ANSI_BOLD "%-22s %-14s %-14s\n" ANSI_COLOR_RESET, "Métrica", "Sequencial", "Paralelo");
    printf("--------------------------------------------------\n");
    printf("%-22s %-14d %-14d\n", "Núcleos Utilizados", 1, num_threads);
    printf("%-22s %-14.11f %-14.11f\n", "Valor Calculado PI", pi_seq, pi_par);
    printf("%-22s %-14.2e %-14.2e\n", "Erro Absoluto", error_seq, error_par);
    printf("%-22s " ANSI_COLOR_YELLOW "%-14.6f" ANSI_COLOR_RESET " " ANSI_COLOR_GREEN "%-14.6f\n" ANSI_COLOR_RESET, "Tempo de Exec. (s)", time_seq, time_par);
    printf("--------------------------------------------------\n");
    
    printf("\n" ANSI_BOLD "Análise de Ganho e Desempenho:\n" ANSI_COLOR_RESET);
    printf("  • Speedup (Aceleração):       " ANSI_COLOR_GREEN ANSI_BOLD "%.2fx" ANSI_COLOR_RESET " (%.1f vezes mais rápido)\n", speedup, speedup);
    printf("  • Eficiência por Núcleo:     " ANSI_COLOR_GREEN ANSI_BOLD "%.2f%%\n" ANSI_COLOR_RESET, efficiency);
    
    if (speedup > 1.1) {
        printf(ANSI_COLOR_GREEN ANSI_BOLD "\n🎉 Sucesso! O processamento paralelo reduziu o tempo de computação.\n\n" ANSI_COLOR_RESET);
    } else {
        printf(ANSI_COLOR_YELLOW "\n⚠ O Speedup foi baixo. Tente com um número maior de iterações (ex: 2.000.000.000) para justificar a sobrecarga de gerenciar as threads.\n\n" ANSI_COLOR_RESET);
    }
    
    return 0;
}
