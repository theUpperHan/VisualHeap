export class BinChunk {
    constructor(type, bin_index, chunk_size, address) {
        this.type = type;
        this.bin_index = bin_index;
        this.chunk_size = chunk_size;
        this.address = address;
    }
}

export class UnsortedChunk {
    constructor(lower_size, upper_size, total_size, count, address) {
        this.lower_size = lower_size;
        this.upper_size = upper_size;
        this.count = count;
        this.total_size = total_size;
        this.address = address
    }
}

export class Heap {
    constructor() {
        this.nr = 0;
        this.lines = [];
        this.fastbins = [];
        this.regularbins = [];
        this.unsorted = [];
        this.total_fast = {};
        this.total_rest = {};
        this.system = {current: 0, max: 0};
        this.aspace = {total: 0, mprotect: 0};
    }

}

export class HeapInfo {
    constructor(ename) {
        this.exename = ename;
        this.heaps = [];
        this.total_fast = {};
        this.total_rest = {};
        this.total_mmap = {};
        this.system = {current: 0, max: 0};
        this.aspace = {total: 0, mprotect: 0};
    }
}


export function fileContentParser(content) {
    const lines = content.split('\n');
    const heapInfo = new HeapInfo(lines[0]);
    let summary_sidx = -1;
    // First pass get each heap lines
    for (let i = 1; i < lines.length; i++) { //TODO: update i?
        // Found a heap
        if (lines[i].startsWith('<heap nr=')) {
            let currentHeap = new Heap();
            currentHeap.nr = parseInt(lines[i].match(/"(.*?)"/)[1]);
            let end = i+1;
            while (end < lines.length) {
                if (lines[end].startsWith('</heap>')) {
                    break;
                }

                // Stores bin lines
                if (lines[end].startsWith('<sizes>')) {
                    currentHeap.lines.push(end+1);
                }
                if (lines[end].startsWith('</sizes>')) {
                    currentHeap.lines.push(end-1);
                }

                // Store single heap statistics
                const stat_pattern = /<(\w+)\s+type="(\w+)"(?:\s+count="(\d+)")?(?:\s+size="(\d+)")?\s*\/>/g;
                const stat_matches = stat_pattern.exec(lines[end]);
                if (stat_matches) {
                    const element = stat_matches[1];
                    const type = stat_matches[2];
                    const count = stat_matches[3] ? parseInt(stat_matches[3]) : null;
                    const size = parseInt(stat_matches[4]);
                    
                    switch (element) {
                        case "total":
                            if (type === "fast") currentHeap.total_fast = {count: count, size: size};
                            else currentHeap.total_rest = {count: count, size: size};
                            break;
                        case "system":
                            if (type === "current") currentHeap.system.current = size;
                            else currentHeap.system.max = size;
                            break;
                        case "aspace":
                            if (type === "total") currentHeap.aspace.total = size;
                            else currentHeap.aspace.mprotect = size;
                            break;
                        default:
                            break;
                    }
                }
                end++;
            }

            const bin_pattern = /\s*<(\w+)\s+index="(\d+)"\s+chunksize="(\d+)"\s+addr="(0x[0-9a-f]+)">/;
            const unsorted_chunk_pattern = /<(\w+)\s+from="(\d+)"\s+to="(\d+)"\s+total="(\d+)"\s+count="(\d+)"\s+addr="(0x[0-9a-f]+)"\s*\/>/;
            for (let i = currentHeap.lines[0]; i <= currentHeap.lines[1]; i++) {
                const bin_matches = bin_pattern.exec(lines[i]);
                const unsorted_matches = unsorted_chunk_pattern.exec(lines[i]);
                if (bin_matches) {
                    const type = bin_matches[1];
                    const index = parseInt(bin_matches[2]);
                    const chunksize = parseInt(bin_matches[3]);
                    const address = bin_matches[4];
                    const bin = new BinChunk(type, index, chunksize, address);
                    if (type === "fast") {
                        currentHeap.fastbins.push(bin);
                    }
                    else {
                        currentHeap.regularbins.push(bin);
                    }
                }
                if (unsorted_matches) {
                    const sfrom = parseInt(unsorted_matches[2]);
                    const sto = parseInt(unsorted_matches[3]);
                    const total_size = parseInt(unsorted_matches[4]);
                    const count = parseInt(unsorted_matches[5]);
                    const address = unsorted_matches[6];
                    const unsorted_chunk = new UnsortedChunk(sfrom, sto, total_size, count, address);
                    currentHeap.unsorted.push(unsorted_chunk);
                }
            }

            // console.log(currentHeap);
            heapInfo.heaps.push(currentHeap);

            i = end; // jump to end of current heap information, i will increment by the for loop
        }

        // lines[i] at this point is always a </heap>
        // if the next is not another heap, then it must be the overall heap statistics
        if (!lines[i+1].startsWith("<heap nr=")) {
            summary_sidx = i+1;
            break;
        }
    }
    // console.log("overall info starts at", summary_sidx);

    if (summary_sidx >= lines.length || summary_sidx === -1) {
        console.error("Error file");
        return null;
    }

    const pattern = /<(\w+)\s+type="(\w+)"(?:\s+count="(\d+)")?\s+size="(\d+)"\s*\/>/;
    for (; summary_sidx < lines.length; summary_sidx++) {
        const stat_matches = pattern.exec(lines[summary_sidx]);
        if (stat_matches) {
            const element = stat_matches[1];
            const type = stat_matches[2];
            const count = stat_matches[3] ? parseInt(stat_matches[3]) : null;
            const size = parseInt(stat_matches[4]);

            switch(element) {
                case "total":
                    if (type === "fast") heapInfo.total_fast = {count: count, size: size};
                    else if (type === "rest") heapInfo.total_rest = {count: count, size: size};
                    else heapInfo.total_mmap = {count: count, size: size};
                    break;
                case "system":
                    if (type === "current") heapInfo.system.current = size;
                    else heapInfo.system.max = size;
                    break;
                case "aspace":
                    if (type === "total") heapInfo.aspace.total = size;
                    else heapInfo.aspace.mprotect = size;
                    break;
                default:
                    break;
            }
        }
    }

    // console.log(heapInfo);
    return heapInfo;
}