# Memória Virtual

Implementação 6 da disciplina de Infraestrutura de Software (CESAR School): gerenciador de memória virtual em C, a partir do *Designing a Virtual Memory Manager* (*Operating System Concepts*, Silberschatz et al., 10ª edição, p. P-51), com memória física de 128 frames, TLB de 16 entradas e substituição FIFO ou LRU, de página e de TLB.

## Requisitos

- Linux, Unix ou macOS (testado apenas em Ubuntu/WSL, com gcc 15.2 e make 4.4)
- Compilador C com suporte a C11 e pthreads
- `make`

## Compilação

```sh
make
```

Gera o executável `vm` na raiz do projeto. Para remover os arquivos compilados:

```sh
make clean
```

## Uso

```sh
./vm <arquivo_de_enderecos> <fifo|lru> <fifo|lru>
```

- 1º argumento: arquivo de endereços lógicos, um inteiro decimal por linha
- 2º argumento: algoritmo de substituição de página
- 3º argumento: algoritmo de substituição da TLB

Exemplo, com página LRU e TLB FIFO:

```sh
./vm addresses.txt lru fifo
```

O programa lê `BACKING_STORE.bin` (256 páginas de 256 bytes) do diretório atual e grava o resultado em `correct.txt`, também no diretório atual, além de mostrar o resumo no terminal. Por isso, rode na pasta que contém o `BACKING_STORE.bin` e nunca na pasta onde está o `correct.txt` de referência, que seria sobrescrito.

Cada endereço de 32 bits é reduzido aos 16 bits da direita: 8 bits de página e 8 de deslocamento.

## Arquivo de endereços

Um inteiro decimal sem sinal (até 4294967295) por linha. São aceitos espaços nas pontas, quebras de linha CRLF e linhas em branco. O arquivo é lido e validado por inteiro antes da simulação.

O programa termina com mensagem de erro e status 1, sem criar `correct.txt`, quando:

- o número de argumentos não é 3, ou um algoritmo não é `fifo` nem `lru` (em minúsculas);
- o arquivo de endereços não existe, não abre, não é um arquivo regular, está vazio ou tem uma linha fora do formato (a mensagem traz o número da linha);
- `BACKING_STORE.bin` não existe no diretório atual, não é um arquivo regular ou não tem 65536 bytes;
- `correct.txt` não pode ser criado.

## Saída

Uma linha por endereço, seguida do resumo:

```
Virtual address: 16916 TLB: 0 Physical address: 20 Value: 0
...
Number of Translated Addresses = 1000
Page Faults = 539
Page Fault Rate = 0.539
TLB Hits = 54
TLB Hit Rate = 0.054
```

`TLB:` é o índice da entrada da TLB usada: a que deu hit ou a que acabou de ser preenchida. Sem TLB (`TLB=off`) a coluna e as duas últimas linhas do resumo não são impressas.

## Configurações de compilação

| Variável | Padrão | Efeito |
|---|---|---|
| `FRAMES` | `128` | número de frames da memória física |
| `TLB` | `on` | `off` traduz só pela tabela de páginas |
| `THREADS` | `on` | `off` busca na TLB em sequência, em vez de uma thread por entrada |

O Makefile recompila tudo quando `FRAMES`, `TLB`, `THREADS`, `CFLAGS` ou `LDFLAGS` mudam. Trocar apenas `CC` exige `make clean`.

Fluxos do enunciado:

```sh
make FRAMES=256 TLB=off      # sem TLB, 256 frames
make TLB=off                 # sem TLB, 128 frames
make THREADS=off             # TLB sem thread
make                         # TLB com thread (padrão)
```

Com os arquivos do professor, `./vm addresses.txt lru fifo` no padrão gera um `correct.txt` idêntico ao de referência (539 faltas, 54 hits).

## Testes

Coloque `addresses.txt`, `correct.txt` e `BACKING_STORE.bin` do professor em `dados/` (a pasta é ignorada pelo git). Sem eles, os testes que dependem dos arquivos do professor são pulados e o `make test` avisa.

```sh
make test        # casos de erro e de saída; usa a configuração de FRAMES, TLB e THREADS
make tsan        # a suíte inteira com ThreadSanitizer
make sanitize    # a suíte inteira com AddressSanitizer e UndefinedBehaviorSanitizer
make mutantes    # quebra o código de propósito e confere que os testes falham
```

`make test FRAMES=256`, `make test TLB=off` e `make test THREADS=off` exercitam os outros fluxos; `make test FRAMES=4` faz a memória encher a toda hora e é o que exercita a invalidação da TLB também com LRU de página. `tests/modelo.awk` é um modelo de referência independente, validado contra o `correct.txt` do professor, que serve de juiz para as combinações sem arquivo de referência.

## Estrutura

- `src/main.c`: orquestra a leitura, a tradução e a gravação do `correct.txt`
- `src/args.c`, `src/policy.c`: linha de comando e algoritmos
- `src/input.c`: leitura e validação do arquivo de endereços
- `src/address.c`: máscara de 16 bits, página, deslocamento e endereço físico
- `src/backing_store.c`: leitura das páginas do `BACKING_STORE.bin`
- `src/vmm.c`: tabela de páginas, frames e substituição de página
- `src/tlb.c`: TLB, substituição e busca com threads
- `src/mmu.c`: TLB, tabela de páginas e invalidação da TLB
- `src/output.c`: formato do `correct.txt`
- `tests/`: `run.sh`, `modelo.awk`, `mutantes.sh` e `conta_threads.c`
