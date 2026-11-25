#!/bin/sh

root=$(pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' INT TERM
total=0
problemas=0

preparar() {
    arquivo=$1
    expressao=$2
    rm -rf "$tmp/m"
    mkdir "$tmp/m"
    cp -r "$root/src" "$root/Makefile" "$tmp/m/"
    sed "$expressao" "$root/src/$arquivo" > "$tmp/m/src/$arquivo"
    if cmp -s "$root/src/$arquivo" "$tmp/m/src/$arquivo"; then
        return 1
    fi
    return 0
}

relatar() {
    nome=$1
    falhas=$2
    if [ "$falhas" -gt 0 ]; then
        printf 'morto  %s (%s testes falharam)\n' "$nome" "$falhas"
    else
        printf 'VIVO   %s\n' "$nome"
        problemas=$((problemas + 1))
    fi
}

mutante() {
    nome=$1
    arquivo=$2
    expressao=$3
    total=$((total + 1))
    if ! preparar "$arquivo" "$expressao"; then
        printf 'ERRO   %s (a expressao nao alterou %s)\n' "$nome" "$arquivo"
        problemas=$((problemas + 1))
        return
    fi
    if ! (cd "$tmp/m" && make -s > /dev/null 2>&1); then
        printf 'ERRO   %s (o mutante nao compilou)\n' "$nome"
        problemas=$((problemas + 1))
        return
    fi
    falhas=$(VM="$tmp/m/vm" sh tests/run.sh 2>&1 | grep -c '^FALHA')
    relatar "$nome" "$falhas"
}

mutante_tsan() {
    nome=$1
    arquivo=$2
    expressao=$3
    total=$((total + 1))
    if ! preparar "$arquivo" "$expressao"; then
        printf 'ERRO   %s (a expressao nao alterou %s)\n' "$nome" "$arquivo"
        problemas=$((problemas + 1))
        return
    fi
    if ! (cd "$tmp/m" && make -s CFLAGS="-std=c11 -g -O1 -fsanitize=thread" LDFLAGS="-fsanitize=thread" > /dev/null 2>&1); then
        printf 'ERRO   %s (o mutante nao compilou com ThreadSanitizer)\n' "$nome"
        problemas=$((problemas + 1))
        return
    fi
    falhas=$(VM="$tmp/m/vm" SANITIZED=thread sh tests/run.sh 2>&1 | grep -c '^FALHA')
    relatar "$nome (ThreadSanitizer)" "$falhas"
}

mutante "FIFO de pagina nao avanca a fila" vmm.c 's/vmm->next_victim = (vmm->next_victim + 1) % FRAME_COUNT;//'
mutante "pagina substituida continua marcada como presente" vmm.c 's/vmm->page_to_frame\[\*evicted\] = -1;//'
mutante "LRU de pagina escolhe o frame mais recente" vmm.c 's/vmm->last_use\[frame\] < vmm->last_use\[victim\]/vmm->last_use[frame] > vmm->last_use[victim]/'
mutante "LRU de pagina se comporta como FIFO" vmm.c 's/vmm->policy == POLICY_FIFO/1/'
mutante "page fault nao e contado" vmm.c 's/vmm->page_faults++;//'
mutante "le a pagina errada da memoria secundaria" vmm.c 's/vmm->store, page, vmm->memory\[found\]/vmm->store, (uint8_t)(page + 1), vmm->memory[found]/'
mutante "endereco fisico com 7 bits de deslocamento" address.c 's/frame << OFFSET_BITS/frame << 7/'
mutante "pagina que perde o frame continua na TLB" mmu.c 's/tlb_invalidate(&mmu->tlb, (uint8_t)evicted);//'
mutante "hit da TLB nao atualiza o uso do frame no LRU" mmu.c 's|vmm_use(&mmu->vmm, mmu->tlb.entries\[slot\].frame, address.offset, access);|{ size_t keep = mmu->vmm.last_use[mmu->tlb.entries[slot].frame]; vmm_use(\&mmu->vmm, mmu->tlb.entries[slot].frame, address.offset, access); mmu->vmm.last_use[mmu->tlb.entries[slot].frame] = keep; }|'
mutante "hit da TLB nao e contado" tlb.c 's/    tlb->hits++;/    tlb->hits += 0;/'
mutante "LRU da TLB escolhe a entrada mais recente" tlb.c 's/tlb->entries\[i\].last_use < tlb->entries\[slot\].last_use/tlb->entries[i].last_use > tlb->entries[slot].last_use/'
mutante "FIFO da TLB nao avanca" tlb.c 's/tlb->next_slot = (tlb->next_slot + 1) % TLB_SIZE;//'
mutante "LRU da TLB ignora entradas livres" tlb.c 's/if (tlb->entries\[i\].page < 0) {/if (0) {/'
mutante "cada thread olha a entrada vizinha" tlb.c 's/tasks\[i\].entry = &tlb->entries\[i\];/tasks[i].entry = \&tlb->entries[(i + 1) % TLB_SIZE];/'
mutante "busca sequencial no lugar das threads" tlb.c 's/return USE_THREADS ? find_threaded(tlb, page) : find_sequential(tlb, page);/return find_sequential(tlb, page);/'
mutante "sem fallback quando a thread nao e criada" tlb.c 's/            search_entry(&tasks\[i\]);//'
mutante "dois valores na mesma linha aceitos" input.c 's/if (parser->number_ended) {/if (0) {/'
mutante "valor acima de 32 bits aceito" input.c 's/if (parser->value > UINT32_MAX) {/if (0) {/'
mutante_tsan "threads sem pthread_join" tlb.c 's/pthread_join(threads\[i\], NULL);//'

printf '%s mutantes, %s com problema\n' "$total" "$problemas"
[ "$problemas" -eq 0 ]
