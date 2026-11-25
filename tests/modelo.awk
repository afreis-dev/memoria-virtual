function victim(    f, best)
{
    best = 0
    for (f = 1; f < frames; f++) {
        if (pagina == "fifo") {
            if (loadf[f] < loadf[best]) {
                best = f
            }
        } else if (lastf[f] < lastf[best]) {
            best = f
        }
    }
    return best
}

function tlb_slot(    i, slot)
{
    if (tlb == "fifo") {
        slot = ring
        ring = (ring + 1) % tlbsize
        return slot
    }
    for (i = 0; i < tlbsize; i++) {
        if (tlbpage[i] == -1) {
            return i
        }
    }
    slot = 0
    for (i = 1; i < tlbsize; i++) {
        if (tlblast[i] < tlblast[slot]) {
            slot = i
        }
    }
    return slot
}

BEGIN {
    if (frames == "") {
        frames = 128
    }
    if (pagina == "") {
        pagina = "lru"
    }
    if (tlb == "") {
        tlb = "none"
    }
    tlbsize = 16
    for (i = 0; i < tlbsize; i++) {
        tlbpage[i] = -1
        tlblast[i] = 0
    }
    if (valores != "") {
        k = 0
        while ((getline x < valores) > 0) {
            vals[++k] = x
        }
    }
}

NF == 0 {
    next
}

{
    total++
    v = $1 % 65536
    page = int(v / 256)
    off = v % 256
    slot = -1
    if (tlb != "none") {
        for (i = 0; i < tlbsize; i++) {
            if (tlbpage[i] == page) {
                slot = i
                break
            }
        }
    }
    if (slot >= 0) {
        hits++
        frame = tlbframe[slot]
        tlblast[slot] = total
        lastf[frame] = total
        field = slot
    } else {
        if (page in pt) {
            frame = pt[page]
            lastf[frame] = total
        } else {
            faults++
            if (used < frames) {
                frame = used++
            } else {
                frame = victim()
                old = fpage[frame]
                delete pt[old]
                for (i = 0; i < tlbsize; i++) {
                    if (tlbpage[i] == old) {
                        tlbpage[i] = -1
                    }
                }
            }
            fpage[frame] = page
            pt[page] = frame
            loadf[frame] = total
            lastf[frame] = total
        }
        field = -1
        if (tlb != "none") {
            slot = tlb_slot()
            tlbpage[slot] = page
            tlbframe[slot] = frame
            tlblast[slot] = total
            field = slot
        }
    }
    if (valores != "") {
        value = vals[total]
    } else {
        value = (page + off) % 256
        if (value > 127) {
            value -= 256
        }
    }
    phys = frame * 256 + off
    if (tlb == "none") {
        printf "Virtual address: %d Physical address: %d Value: %d\n", v, phys, value
    } else {
        printf "Virtual address: %d TLB: %d Physical address: %d Value: %d\n", v, field, phys, value
    }
}

END {
    printf "Number of Translated Addresses = %d\n", total
    printf "Page Faults = %d\n", faults
    printf "Page Fault Rate = %.3f\n", faults / total
    if (tlb != "none") {
        printf "TLB Hits = %d\n", hits
        printf "TLB Hit Rate = %.3f\n", hits / total
    }
}
