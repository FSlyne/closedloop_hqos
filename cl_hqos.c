#include <stdio.h>
#include <stdlib.h>


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

void dotproduct(float X[], float Y[], float *R, int L) {
    *R=0.0;
    for (int i=0; i<L; i++){
        *R+=X[i]*Y[i];  
    }
    return;
}

void scalmatmult(float X[], float R, float Y[], float * Tot, int L) {
    *Tot=0;
    for (int i=0; i<L; i++){
        Y[i] = X[i] * R;
        *Tot+=Y[i];
    }
    return;
}

void sigma(float X[], float *R, int L){
    *R=0.0;
    for (int i=0; i<L; i++){
        *R+=X[i];
    }
    return;
}

void delta(float X[], float Y[], float Z[], int L){
    for (int i=0; i<L; i++){
        Z[i]=X[i]-Y[i];
    }
    return;
}

void calc_weight(float X[], float Y[], int L) {
    float R=0.0;
    for (int i=0; i<L; i++) {
        R+=X[i];
    }
    if (R== 0) {
        for (int i=0; i<L; i++) {
            Y[i]=0;
        }
    } else {
        for (int i=0; i<L; i++) {
            Y[i]=X[i]/R;
        }
    return;
    }
}


void rebalance(float N[], float Wtin[], float Wtout[], int L) {
    float xs = 0, nxs =0;
    for (int i=0; i<L; i++) {
        if (N[i] == 0) {
            xs+= Wtin[i];
            Wtout[i]=0;
        } else {
            Wtout[i]=Wtin[i];
            nxs+= Wtin[i];
        }
    }
    if (nxs > 0 ){
        for (int i=0; i<L; i++) {
            Wtout[i] += Wtout[i]*xs/nxs;
        }
        return;
    }
    return;
}

void trtcm(float rate, float CIR, float EIR, float *Green, float *Yellow, float *XSGreen, float *XSYellow){
    *Yellow=0.0; *Green=0.0; 
    if (rate<CIR) {
        *Green=rate;
    } else {
        *Green=CIR;
        rate=rate-CIR;
        if (rate > EIR) {
            *Yellow=EIR;
        } else {
            *Yellow=rate;
        }
    }
    *XSGreen=CIR-*Green;
    *XSYellow=EIR-*Yellow;
    return;
}

void qos_broker(float N[], float Wtin[], float XSin, float Remain[], float * XSout, int L) {
    float Wttmp[L];
    float TotRemain;
    rebalance(N, Wtin, Wttmp, L);
    for (int i=0; i<L; i++){
        printf("Total2 :%.3f %.3f\n", Wtin[i], Wttmp[i] );
    }   
    scalmatmult(Wttmp, XSin, Remain, &TotRemain, L);
    *XSout=max(0,XSin-TotRemain);
}

 typedef struct  {
    float CIR;
    float EIR;
} ONT;

void ont_init(ONT* self, float CIR, float EIR ) {
    self->CIR=CIR;
    self->EIR=EIR;
}

ONT* ont_create(float CIR, float EIR){
    ONT* obj=(ONT*) malloc(sizeof(ONT));
    ont_init(obj, CIR, EIR);
    return obj;
}

void ont_destroy(ONT* obj){
    if (obj) {
        free(obj);
    }
}

void ont_update(ONT* self, float ZiHP, float ZiLP, float * XiHP, float * XiLP,
             float *XSG, float *XSY, float RiHP, float RiLP){
    float C, E;
    float XSGreen1=0.0, XSYellow1=0.0;
    float GiHP=0.0, YiHP=0.0, GiLP=0.0, YiLP=0.0;
    C=self->CIR+RiHP-*XSG;
    E=self->EIR+RiLP-*XSY;  // Iterative Calculations
    trtcm(ZiHP, C, E, &GiHP, &YiHP, &XSGreen1, &XSYellow1);
    trtcm(ZiLP, XSGreen1, XSYellow1, &GiLP, &YiLP, XSG, XSY);
    *XiHP=GiHP+YiHP;
    *XiLP=GiLP+YiLP;
}


typedef struct {
    ONT* ont[4];
    float Wtin[4];
    int ONTnum;
} VNO;


void vno_init(VNO* self, float CIR[], float PIR[], float linerate, int ONTnum ) {
    float E=linerate;
    self->ONTnum=ONTnum;
    for (int i=0; i<ONTnum; i++){
        E-=CIR[i];
    }
    
    calc_weight(CIR, self->Wtin, ONTnum);
    for (int i=0; i<ONTnum; i++) {
        self->ont[i]=ont_create(CIR[i],E*self->Wtin[i]);
    }
}

VNO* vno_create(float CIR[], float PIR[], float linerate, int ONTnum){
    VNO* obj=(VNO*) malloc(sizeof(VNO));
    vno_init(obj, CIR, PIR, linerate, ONTnum);
    return obj;
}

void vno_destroy(VNO* self){
    for (int i=0; i<self->ONTnum; i++) {
        ont_destroy(self->ont[i]);
    }
    if (self) {
        free(self);
    }
}

void vno_update(VNO* self, float ZiHP[], float ZiLP[], float XiHP[], float XiLP[], float XSin, float *XSout){
    float needs[] = {0,0,0,0};
    float allocated[] = {0,0,0,0};
    float XSG[]={0,0,0,0}, XSY[]={0,0,0,0};
    float XS[]={0,0,0,0};
    float XStot=0.0;


    for (int j=0; j<2; j++) {
         XStot=0;
    for (int i=0; i<self->ONTnum; i++) {
      ont_update(self->ont[i], ZiHP[i], ZiLP[i], &XiHP[i], &XiLP[i], &XSG[i], &XSY[i], 0, allocated[i]);
      needs[i]=ZiHP[i]+ZiLP[i]-(XiHP[i]+XiLP[i]);
      XS[i]=XSG[i]+XSY[i];
      XStot+=XS[i];      
    }
    qos_broker(needs, self->Wtin, XStot+XSin, allocated, XSout, self->ONTnum);
    }
    return;
}


int main () {

    float ZiHP[] = {12,10,8,6};
    float ZiLP[] = {4,4,4,12};
    float XiHP[] = {0,0,0,0};
    float XiLP[] = {0,0,0,0};

    int linerate=50;
    
    float CIR[]={10,10,10,10};
    float PIR[]={100,100,100,100};
    int ONTnum=sizeof(CIR)/sizeof(float);
    VNO* vno = vno_create(CIR, PIR, linerate, ONTnum);
    float XSin=0, XSout=0;
    vno_update(vno, ZiHP, ZiLP, XiHP, XiLP, XSin, &XSout);
    
    for (int i=0; i<ONTnum; i++){
        printf("Xi :%.3f %.3f\n", XiHP[i], XiLP[i] );
    }   
}