# 🧮 Cálculo Paralelo de $\pi$ com OpenMP

Este é um projeto em **C** que calcula o valor da constante matemática $\pi$ (Pi) utilizando o método de integração numérica (Soma de Riemann). A aplicação compara o desempenho do cálculo executado de forma sequencial com a execução paralela utilizando a biblioteca **OpenMP** (Open Multi-Processing).

O grande diferencial deste projeto é a sua interface no terminal, que apresenta **barras de progresso coloridas em tempo real** para cada uma das threads ativas, além de exibir estatísticas detalhadas de desempenho (*Speedup* e Eficiência).

---

## 🚀 Funcionalidades

- **Cálculo de $\pi$ por Integração**: Calcula o valor aproximado de $\pi$ utilizando a fórmula de aproximação numérica:
  $$\int_{0}^{1} \frac{4}{1 + x^2} dx = \pi$$
- **Execução Interativa**: Permite que o usuário defina o número total de iterações e a quantidade de threads/núcleos a serem utilizados diretamente pelo terminal (com valores padrões sugeridos).
- **Monitoramento em Tempo Real (Thread de Monitoramento)**: Utiliza uma thread adicional exclusiva para renderizar no terminal barras de progresso dinâmicas em tempo real (atualizadas a ~30ms / 33 FPS) sem bloquear ou interferir no desempenho do cálculo matemático.
- **Relatório de Desempenho**: Exibe uma tabela com o tempo de execução de ambas as abordagens, o erro absoluto em relação ao valor real de $\pi$, o ganho de velocidade (*Speedup*) e a eficiência de processamento.

---

## 🛠️ Pré-requisitos

O `Makefile` está configurado para **Linux**, utilizando o `gcc` com suporte a OpenMP (`-fopenmp`). O suporte a OpenMP já acompanha o `gcc` (via `libgomp`), então basta instalar o compilador e o `make`.

### 🐧 Linux

- **Debian / Ubuntu**:
  ```bash
  sudo apt update
  sudo apt install build-essential
  ```

- **Fedora / RHEL / CentOS**:
  ```bash
  sudo dnf install gcc make
  ```

- **Arch Linux**:
  ```bash
  sudo pacman -S base-devel
  ```

Para verificar a instalação:
```bash
gcc --version
make --version
```

### 🍎 macOS (alternativa)

No macOS é necessário usar o `clang` com a biblioteca `libomp` via Homebrew, e ajustar `CC` e as flags no `Makefile`:

```bash
xcode-select --install
brew install libomp
```

*(No Windows, recomenda-se utilizar o WSL com uma distribuição Linux e seguir as instruções acima.)*

---

## ⚙️ Como Compilar e Executar

O projeto inclui um `Makefile` configurado para automatizar as tarefas.

### 1. Compilar o Projeto
Execute o comando abaixo no terminal da raiz do projeto:
```bash
make
```
Isso gerará o binário executável `parallel_pi`.

Se preferir compilar manualmente, sem o `make`:
```bash
gcc -O3 -fopenmp main.c -o parallel_pi -lm
```

### 2. Executar a Aplicação
Inicie a aplicação executando:
```bash
./parallel_pi
```

O programa solicita interativamente o número de iterações e de threads (Enter usa o valor padrão). Para descobrir quantos núcleos lógicos sua máquina possui no Linux:
```bash
nproc
```

### 3. Limpar os Arquivos Temporários
Para remover o executável gerado:
```bash
make clean
```

---

## 🖥️ Exemplo de Uso

Ao iniciar o programa, ele detecta automaticamente o número de núcleos lógicos do seu processador e solicita os parâmetros de execução:

```text
==================================================
     CÁLCULO PARALELO DE PI COM OPENMP            
==================================================
Núcleos (Threads) lógicos disponíveis no sistema: 20

Informe o número de iterações [1.000.000.000 - Enter para padrão]: 
Informe o número de núcleos (threads) [8 - Enter para padrão]: 

[1/2] Iniciando cálculo SEQUENCIAL...
Sequencial: [██████████████████████████████] 100.0%
✔ Sequencial concluído em 2.112509 segundos.

[2/2] Iniciando cálculo PARALELO utilizando 8 núcleos...
Núcleo  0: [██████████████████████████████] 100.0%
Núcleo  1: [██████████████████████████████] 100.0%
Núcleo  2: [██████████████████████████████] 100.0%
Núcleo  3: [██████████████████████████████] 100.0%
Núcleo  4: [██████████████████████████████] 100.0%
Núcleo  5: [██████████████████████████████] 100.0%
Núcleo  6: [██████████████████████████████] 100.0%
Núcleo  7: [██████████████████████████████] 100.0%
✔ Paralelo concluído em 0.373184 segundos.

==================================================
                TABELA DE RESULTADOS              
==================================================
Métrica                Sequencial     Paralelo      
--------------------------------------------------
Núcleos Utilizados    1              8             
Valor Calculado PI     3.14159265359  3.14159265359 
Erro Absoluto          1.78e-13       2.40e-14      
Tempo de Exec. (s)     2.112509       0.373184      
--------------------------------------------------

Análise de Ganho e Desempenho:
  • Speedup (Aceleração):       5.66x (5.7 vezes mais rápido)
  • Eficiência por Núcleo:     70.76%

🎉 Sucesso! O processamento paralelo reduziu o tempo de computação.
```

---

## 🧠 Detalhes de Implementação

### Estrutura Multithread
Para garantir a fidelidade do tempo de execução do algoritmo paralelo, o projeto usa um modelo assíncrono para a interface gráfica no terminal:
- **$N$ Worker Threads**: Executam as iterações matemáticas. O intervalo de iterações total é dividido igualmente entre elas. Cada thread atualiza de forma atômica o seu respectivo progresso em uma variável em memória (`thread_progress`).
- **1 Monitor Thread**: Uma thread dedicada exclusivamente a redesenhar as barras de progresso do terminal. Ela consome os dados de progresso em tempo real e os atualiza usando códigos ANSI de movimentação de cursor. Ela finaliza quando todas as worker threads concluem seu trabalho.

### Tecnologias Utilizadas
- **Linguagem C**
- **OpenMP API** (Pragmas `#pragma omp parallel` e operações atômicas `#pragma omp atomic`)
- **Códigos de escape ANSI** para interface gráfica colorida dinâmica no terminal.
