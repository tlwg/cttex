#define PORPAR 187
#define FORFAR 189
#define FORFUN 191
#define MAITAI 231
#define MAIHAN 209
#define SARAAMP 211
#define YORYING 173
#define isyol(x) ((x)==PORPAR||(x)==FORFAR||(x)==FORFUN)

void adj(line)
unsigned char *line;
{
    unsigned char top[MAXLINELENGTH];
    unsigned char up[MAXLINELENGTH];
    unsigned char middle[MAXLINELENGTH];
    unsigned char low[MAXLINELENGTH];

    int i, k, c;

    /* Split string into 4 levels */
    
    /* Clear Buffer */
    for(i=0;i<MAXLINELENGTH;i++)
	top[i]=up[i]=middle[i]=low[i]=0;

    i=0; k=-1;
    while((c=line[i++])!=0) {
	switch((c>0xD0)?levtable[c-0xD0]:0){
	case 0 : /*Middle*/
	    k++;
	    middle[k]=c; break;
	case 1 : /*Low*/
	    low[k]=c; break;
	case 2 : /*Up*/
	    up[k]=c; break;
	case 3 : /*Top*/
	    top[k]=c; break;
	}
    }

    /* Beauty Part */
    /* Check through each condition */
    for(i=0;middle[i]!='\n';i++) {
	if(isyol(middle[i])&&middle[i+1]!=SARAAMP) {
	    if(up[i]!=0){
		if(up[i]!=MAIHAN&&up[i]!=MAITAI)
		    up[i]=up[i]-64; /*SARA for PORPAR*/
                if(up[i]==MAITAI)
		    up[i]=up[i]-84; /*MAITAIKOOL for PORPAR*/
		if(up[i]==MAIHAN)
		    up[i]=up[i]-63; /*MAIHANAREKARD for PORPAR*/
		if(top[i]!=0)
		    top[i]=top[i]-80; /*MAIEK for PORPAR and SARA*/
	    }else{
		if(top[i]!=0)
		    top[i]=top[i]-101; /*MAIEK for PORPAR*/
	    }
	}else{
	    if(top[i]!=0&&up[i]==0&&middle[i+1]!=SARAAMP)
		top[i]=top[i]-96; /*MAIEK for BORBAIMAI*/
	}
	if(middle[i]==YORYING&&low[i]!=0)
	    middle[i]=144; /*YORYING for SARAUOO*/
	if(middle[i+1]==SARAAMP&&top[i]!=0)
	    top[i]=top[i]-80; /*MAIEK for SARAAMP*/
    }
    
    /* Pack Back To Line */
    
    i=0; k=0;
    while(middle[i]){
	line[k++]=middle[i];
	if(low[i]) line[k++]=low[i];
	if(up[i])  line[k++]=up[i];
	if(top[i]) line[k++]=top[i];
	i++;
    }
}
