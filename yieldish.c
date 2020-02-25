#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int pktcnt;
    int status;
} PKT;

void pkt_init(PKT* self) {
    self->pktcnt=0;
    self->status=0;
}

PKT* pkt_create() {
    PKT* obj=(PKT*) malloc(sizeof(PKT));
    pkt_init(obj);
    return obj;
}

void pkt_get(PKT* self, int *ret) {
    if (self->status == 0) {
        self->status = 1;
    }
    // preprocess
    *ret=(self->pktcnt)++;
    // postprocess
    return;
}

int main() {
    PKT* pkt=pkt_create();
    int pktres;
    
    for (int i=0; i<20; i++) {
        pkt_get(pkt, &pktres);
        printf("Processing packet %d\n", pktres);
    }
}
