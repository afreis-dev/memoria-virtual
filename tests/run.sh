#!/bin/sh

VM=${VM:-./vm}
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

expect_ok() {
    name=$1
    pattern=$2
    shift 2
    out=$("$@" 2>&1)
    code=$?
    if [ "$code" -eq 0 ] && printf '%s\n' "$out" | grep -qF -- "$pattern"; then
        pass "$name"
    else
        fail "$name (status=$code, saida: $out)"
    fi
}

expect_stdout() {
    name=$1
    expected=$2
    shift 2
    "$@" > "$tmp/stdout.txt" 2>/dev/null
    code=$?
    if [ "$code" -eq 0 ] && cmp -s "$tmp/stdout.txt" "$expected"; then
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
printf 'Virtual address: 0 Page: 0 Offset: 0\n' > "$tmp/traducao-esperada.txt"
printf 'Virtual address: 255 Page: 0 Offset: 255\n' >> "$tmp/traducao-esperada.txt"
printf 'Virtual address: 256 Page: 1 Offset: 0\n' >> "$tmp/traducao-esperada.txt"
printf 'Virtual address: 65535 Page: 255 Offset: 255\n' >> "$tmp/traducao-esperada.txt"
printf 'Virtual address: 0 Page: 0 Offset: 0\n' >> "$tmp/traducao-esperada.txt"
printf 'Virtual address: 4464 Page: 17 Offset: 112\n' >> "$tmp/traducao-esperada.txt"
printf 'Virtual address: 65535 Page: 255 Offset: 255\n' >> "$tmp/traducao-esperada.txt"

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

expect_ok "enderecos validos" "Virtual address: 30198 Page: 117 Offset: 246" "$VM" "$tmp/valido.txt" lru fifo
expect_ok "espacos, CRLF e linha em branco" "Virtual address: 30 Page: 0 Offset: 30" "$VM" "$tmp/espacos.txt" fifo lru
expect_ok "limites de 32 bits" "Virtual address: 65535 Page: 255 Offset: 255" "$VM" "$tmp/limites.txt" fifo fifo

expect_stdout "traducao: bordas e mascara de 16 bits" "$tmp/traducao-esperada.txt" "$VM" "$tmp/traducao.txt" lru fifo
if [ -f dados/addresses.txt ] && [ -f dados/correct.txt ]; then
    "$VM" dados/addresses.txt lru fifo | awk '{print $3, $5, $7}' > "$tmp/obtido.txt"
    awk '/^Virtual/ {print $3, int($3 / 256), $8 % 256}' dados/correct.txt > "$tmp/esperado.txt"
    if [ -s "$tmp/obtido.txt" ] && cmp -s "$tmp/obtido.txt" "$tmp/esperado.txt"; then
        pass "traducao: pagina e deslocamento conferem com o correct.txt"
    else
        fail "traducao: pagina e deslocamento diferem do correct.txt"
    fi
fi

if [ "$status" -eq 0 ]; then
    printf 'todos os testes passaram\n'
fi
exit "$status"
