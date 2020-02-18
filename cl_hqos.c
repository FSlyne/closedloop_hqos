#include <stdio.h>

void dotproduct(float X[], float Y[], float *R, int L) {
    *R=0.0;
    for (int i=0; i<L; i++){
        *R+=X[i]*Y[i];  
    }
    return;
}

void scalmatmult(float X[], float R, float Y[], int L) {
    for (int i=0; i<L; i++){
        Y[i] = X[i] * R;
        printf("Total1 :%.f\n", X[i]);
    }
    return;
}

void sigma(float X[], float *R, int L){
    *R=0.0;
    for (int i; i<L; i++){
        *R+=X[i];
    }
    return;
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
    float privrate=rate;
    *Yellow=0.0; *Green=0.0; 
    if (privrate<CIR) {
        *Green=privrate;
    } else {
        *Green=CIR;
        privrate=privrate-CIR;
        if (privrate > EIR) {
            *Yellow=EIR;
        } else {
            *Yellow=privrate;
        }
    }
    *XSGreen=CIR-*Green;
    *XSYellow=EIR-*Yellow;
    return;
}

void ONT_QoS(float ZiHP, float ZiLP, float * XiHP, float * XiLP,
             float *XSG, float *XSY, float CIR, float EIR, float RiHP, float RiLP) {
    float C, E;
    float XSGreen1=0.0, XSYellow1=0.0;
    float GiHP=0.0, YiHP=0.0, GiLP=0.0, YiLP=0.0;
    C=CIR+RiHP-*XSG; E=EIR+RiLP-*XSY;  // Iterative Calculations
    trtcm(ZiHP, C, E, &GiHP, &YiHP, &XSGreen1, &XSYellow1);
    trtcm(ZiLP, XSGreen1, XSYellow1, &GiLP, &YiLP, XSG, XSY);
    *XiHP=GiHP+YiHP;
    *XiLP=GiLP+YiLP;
}

void qos_calc(float N[], float Wtin[], float XStot, float Remain[], int L) {
    float Wttmp[L];
    rebalance(N, Wtin, Wttmp, L);
    for (int i=0; i<L; i++){
        printf("Total2 :%.3f %.3f\n", Wtin[i], Wttmp[i] );
    }   
    scalmatmult(Wttmp, XStot, Remain, L);
}

void PON(float ZiHP[], float ZiLP[], float XiHP[], float XiLP[], float Remain[], float Linerate, int L) {
    float N[] = {0,0,0,0};
    float Wtin[] = {0.25,0.25,0.25,0.25};
    float XSG[]={0,0,0,0}, XSY[]={0,0,0,0};
    float XStot=0.0;
    float CIR[]={10,10,10,10};
    float PIR[]={100,100,100,100};
    float EIR[]={0,0,0,0};
    float E=Linerate;
    
    for (int i=0; i<L; i++){
        EIR[i] = PIR[i]-CIR[i];
        E-=CIR[i];
    }
    
    for (int j=0; j<2; j++) {
        XStot=0;
        for (int i=0; i<L; i++){
            ONT_QoS(ZiHP[i], ZiLP[i], &XiHP[i], &XiLP[i], &XSG[i], &XSY[i],
                    CIR[i], E*Wtin[i], 0, Remain[i] );
            N[i]=ZiHP[i]+ZiLP[i]-(XiHP[i]+XiLP[i]);
            XStot+=XSG[i]+XSY[i];
        }
        qos_calc(N, Wtin, XStot, Remain, L);
    }
    return;
}



int main()
{
    
    float ZiHP[] = {8,8,8,8};
    float ZiLP[] = {8,8,8,12};
    float XiHP[] = {0,0,0,0};
    float XiLP[] = {0,0,0,0};
    float Remain[] = {0,0,0,0};
    int L=sizeof(ZiHP)/sizeof(float);
    
    PON(ZiHP, ZiLP, XiHP, XiLP, Remain, 50, L);

    for (int i=0; i<L; i++){
        printf("Total3 :%.3f %.3f\n", XiHP[i], XiLP[i] );
    }   
}