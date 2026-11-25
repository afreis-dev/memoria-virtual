#!/bin/sh

root=$(pwd)
VM=${VM:-$root/vm}
dados=$root/dados
status=0
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' INT TERM

pass() {
    printf 'ok     %s\n' "$1"
}

fail() {
    printf 'FALHA  %s\n' "$1"
    status=1
}

in_dir() {
    dir=$1
    shift
    (cd "$dir" && "$@")
}

expect_error() {
    name=$1
    pattern=$2
    shift 2
    out=$("$@" 2>&1 >/dev/null)
    code=$?
    if [ "$code" -eq 1 ] && printf '%s\n' "$out" | grep -qF -- "$pattern"; then
        pass "$name"
    else
        fail "$name (status=$code, stderr: $out)"
    fi
}

make_backing() {
    LC_ALL=C awk 'BEGIN { for (i = 0; i < 65536; i++) printf "%c", (int(i / 256) + i) % 256 }' > "$1"
}

expected_output() {
    awk '
    NF == 0 { next }
    {
        v = $1 % 65536
        page = int(v / 256)
        off = v % 256
        if (!(page in frame)) {
            frame[page] = used++
            faults++
        }
        value = (page + off) % 256
        if (value > 127) {
            value -= 256
        }
        printf "Virtual address: %d Physical address: %d Value: %d\n", v, frame[page] * 256 + off, value
        total++
    }
    END {
        printf "Number of Translated Addresses = %d\n", total
        printf "Page Faults = %d\n", faults
        printf "Page Fault Rate = %.3f\n", faults / total
    }' "$1"
}

expect_translation() {
    name=$1
    input=$2
    rm -f correct.txt
    "$VM" "$input" lru fifo > /dev/null 2>&1
    code=$?
    expected_output "$input" > "$tmp/esperado-saida.txt"
    if [ "$code" -eq 0 ] && cmp -s correct.txt "$tmp/esperado-saida.txt"; then
        pass "$name"
    else
        fail "$name (status=$code)"
    fi
}

expect_total() {
    name=$1
    input=$2
    total=$3
    rm -f correct.txt
    "$VM" "$input" fifo lru > /dev/null 2>&1
    code=$?
    if [ "$code" -eq 0 ] && grep -qx "Number of Translated Addresses = $total" correct.txt; then
        pass "$name"
    else
        fail "$name (status=$code)"
    fi
}

printf '16916\n62493\n30198\n' > "$tmp/valido.txt"
printf '  10 \r\n20\t\r\n\n30' > "$tmp/espacos.txt"
printf '4294967295\n0\n' > "$tmp/limites.txt"
: > "$tmp/vazio.txt"
printf '\n  \n\n' > "$tmp/branco.txt"
printf '1\nabc\n3\n' > "$tmp/letras.txt"
printf '1\n-5\n' > "$tmp/negativo.txt"
printf '1\n+5\n' > "$tmp/positivo.txt"
printf '1\n12.5\n' > "$tmp/decimal.txt"
printf '1\n12 34\n' > "$tmp/dois.txt"
printf '0x10\n' > "$tmp/hex.txt"
printf '4294967296\n' > "$tmp/grande.txt"
printf '12\000\n' > "$tmp/nulo.txt"
printf '12abc\n' > "$tmp/misto.txt"
printf '0\n255\n256\n65535\n65536\n70000\n4294967295\n' > "$tmp/traducao.txt"
printf '100\n100\n356\n100\n300\n356\n65280\n' > "$tmp/repetidos.txt"

mkdir "$tmp/work" "$tmp/semstore" "$tmp/curto" "$tmp/dirstore" "$tmp/saida"
make_backing "$tmp/work/BACKING_STORE.bin"
head -c 1000 "$tmp/work/BACKING_STORE.bin" > "$tmp/curto/BACKING_STORE.bin"
mkdir "$tmp/dirstore/BACKING_STORE.bin"
cp "$tmp/work/BACKING_STORE.bin" "$tmp/saida/"
mkdir "$tmp/saida/correct.txt"
cd "$tmp/work" || exit 1

expect_error "sem argumentos" "numero de argumentos invalido" "$VM"
expect_error "um argumento" "numero de argumentos invalido" "$VM" "$tmp/valido.txt"
expect_error "dois argumentos" "numero de argumentos invalido" "$VM" "$tmp/valido.txt" lru
expect_error "quatro argumentos" "numero de argumentos invalido" "$VM" "$tmp/valido.txt" lru fifo extra
expect_error "caminho vazio" "caminho do arquivo de enderecos esta vazio" "$VM" "" lru fifo
expect_error "algoritmo de pagina invalido" "pagina invalido 'xyz'" "$VM" "$tmp/valido.txt" xyz fifo
expect_error "algoritmo da TLB invalido" "TLB invalido 'abc'" "$VM" "$tmp/valido.txt" lru abc
expect_error "algoritmo em maiusculas" "invalido 'LRU'" "$VM" "$tmp/valido.txt" LRU fifo

expect_error "arquivo inexistente" "nao encontrado" "$VM" "$tmp/naoexiste.txt" lru fifo
expect_error "diretorio no lugar do arquivo" "nao e um arquivo regular" "$VM" "$tmp" lru fifo
if [ "$(id -u)" -ne 0 ]; then
    : > "$tmp/sem-permissao.txt"
    chmod 000 "$tmp/sem-permissao.txt"
    expect_error "arquivo sem permissao de leitura" "nao foi possivel abrir" "$VM" "$tmp/sem-permissao.txt" lru fifo
fi
expect_error "arquivo vazio" "nao contem nenhum endereco" "$VM" "$tmp/vazio.txt" lru fifo
expect_error "somente linhas em branco" "nao contem nenhum endereco" "$VM" "$tmp/branco.txt" lru fifo
expect_error "texto no lugar do numero" "linha 2: caractere invalido 'a'" "$VM" "$tmp/letras.txt" lru fifo
expect_error "numero negativo" "linha 2: caractere invalido '-'" "$VM" "$tmp/negativo.txt" lru fifo
expect_error "sinal positivo" "linha 2: caractere invalido '+'" "$VM" "$tmp/positivo.txt" lru fifo
expect_error "numero decimal" "linha 2: caractere invalido '.'" "$VM" "$tmp/decimal.txt" lru fifo
expect_error "dois valores na mesma linha" "linha 2: mais de um valor" "$VM" "$tmp/dois.txt" lru fifo
expect_error "hexadecimal" "linha 1: caractere invalido 'x'" "$VM" "$tmp/hex.txt" lru fifo
expect_error "valor acima de 32 bits" "linha 1: valor fora do intervalo" "$VM" "$tmp/grande.txt" lru fifo
expect_error "byte nulo" "linha 1: caractere invalido 0x00" "$VM" "$tmp/nulo.txt" lru fifo
expect_error "numero seguido de texto" "linha 1: caractere invalido 'a'" "$VM" "$tmp/misto.txt" lru fifo

expect_error "memoria secundaria ausente" "BACKING_STORE.bin nao encontrado" in_dir "$tmp/semstore" "$VM" "$tmp/valido.txt" lru fifo
expect_error "memoria secundaria com tamanho errado" "deve ter 65536 bytes, mas tem 1000" in_dir "$tmp/curto" "$VM" "$tmp/valido.txt" lru fifo
expect_error "memoria secundaria e um diretorio" "nao e um arquivo regular" in_dir "$tmp/dirstore" "$VM" "$tmp/valido.txt" lru fifo
expect_error "arquivo de saida nao pode ser criado" "nao foi possivel criar correct.txt" in_dir "$tmp/saida" "$VM" "$tmp/valido.txt" lru fifo

rm -f correct.txt
"$VM" "$tmp/naoexiste.txt" lru fifo > /dev/null 2>&1
if [ ! -e correct.txt ]; then
    pass "entrada invalida nao cria o correct.txt"
else
    fail "entrada invalida criou o correct.txt"
fi

expect_translation "saida: enderecos validos" "$tmp/valido.txt"
expect_translation "saida: bordas e mascara de 16 bits" "$tmp/traducao.txt"
expect_translation "saida: paginas repetidas reaproveitam o frame" "$tmp/repetidos.txt"
expect_total "espacos, CRLF e linha em branco" "$tmp/espacos.txt" 3
expect_total "limites de 32 bits" "$tmp/limites.txt" 2

if [ -f "$dados/addresses.txt" ] && [ -f "$dados/correct.txt" ] && [ -f "$dados/BACKING_STORE.bin" ]; then
    mkdir "$tmp/prof"
    cp "$dados/BACKING_STORE.bin" "$tmp/prof/"
    in_dir "$tmp/prof" "$VM" "$dados/addresses.txt" lru fifo > /dev/null 2>&1
    saida=$tmp/prof/correct.txt

    paginas=$(awk '{print int($1 / 256)}' "$dados/addresses.txt" | sort -u | wc -l | tr -d ' ')
    if grep -qx "Page Faults = $paginas" "$saida"; then
        pass "professor: page faults iguais ao numero de paginas distintas ($paginas)"
    else
        fail "professor: page faults diferentes de $paginas"
    fi

    awk '/^Virtual/ {print $3, $8}' "$saida" > "$tmp/obtido.txt"
    awk '/^Virtual/ {print $3, $10}' "$dados/correct.txt" > "$tmp/esperado.txt"
    if [ -s "$tmp/obtido.txt" ] && cmp -s "$tmp/obtido.txt" "$tmp/esperado.txt"; then
        pass "professor: coluna Value confere com o correct.txt"
    else
        fail "professor: coluna Value difere do correct.txt"
    fi

    awk '/^Virtual/ {print $3, $6 % 256}' "$saida" > "$tmp/obtido.txt"
    awk '/^Virtual/ {print $3, $8 % 256}' "$dados/correct.txt" > "$tmp/esperado.txt"
    if [ -s "$tmp/obtido.txt" ] && cmp -s "$tmp/obtido.txt" "$tmp/esperado.txt"; then
        pass "professor: deslocamento confere com o correct.txt"
    else
        fail "professor: deslocamento difere do correct.txt"
    fi

    limite=$(awk '/^Virtual/ { p = int($3 / 256); if (!(p in v)) { v[p] = 1; if (++n == 129) { print NR - 1; exit } } }' "$dados/correct.txt")
    head -n "$limite" "$saida" | awk '{print $3, $6}' > "$tmp/obtido.txt"
    head -n "$limite" "$dados/correct.txt" | awk '{print $3, $8}' > "$tmp/esperado.txt"
    if [ -s "$tmp/obtido.txt" ] && cmp -s "$tmp/obtido.txt" "$tmp/esperado.txt"; then
        pass "professor: enderecos fisicos conferem ate a primeira substituicao (linha $limite)"
    else
        fail "professor: enderecos fisicos diferem antes da primeira substituicao"
    fi
fi

if [ "$status" -eq 0 ]; then
    printf 'todos os testes passaram\n'
fi
exit "$status"
