/* Thai word-separator by dictionary          */
/* By Vuthichai A.                            */
/* vuthi@ctrl.titech.ac.jp                    */
/* Change Log is available at the end of file */

/* $Header: /home/cvs/software/cttex/old/cttex116b.c,v 1.1 2001-02-28 12:31:40 chanop Exp $ 
*/

/* Maximum number of words in the dictionary */
#define                 MAXWORD                 20000

/* Maximum length of one word in the dict */
#define                 MAXWORDLENGTH           40

/* Maximum length of input line */
#define                 MAXLINELENGTH           1000

/* Maximum number of WORDS in one line */
#define                 MW                      60

/* Maximum number of words to LOOKBACK */
#define                 BACKDEPTH               3

/* Characters to be skipped */
#define                 SKIPWORD(x)             \
                        (((x)<128) || (((x)<=0xF9)&&((x)>=0xF0)))

/* HIGH Chars */
#define                 HIGHWORD(x)             \
                        (((x)>=128))

/* Check level of a character */
#define                 NOTMIDDLE(x)            \
                        ((x)<0xD0?0:(levtable[(x)-0xD0]!=0))

/* Never change this value. If you do, make sure it's below 255. */
#define			CUTCODE			254

/* Set this one will reduce output size with new TeX */
#define                 HIGHBIT                 1

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

/* Load Dictionary : wordptr & numword */
#include "tdict.h"

void dooneline(unsigned char *,unsigned char *);
void savestatus(int*, int*, int*, int*, int*, int*, unsigned char *, int);
void adj(unsigned char *);
void filter(unsigned char *);
void fixline(unsigned char *);
int mystrncmp(unsigned char *, unsigned char *, int);
int findword(unsigned char *, int *);
int countmatch(unsigned char *in, unsigned char *out);
int moveleft(int);

/* Table Look-Up for level of a character */
/* only those in the range D0-FF */
int levtable[]={
                0,2,0,0,2,2,2,2,1,1,1,0,0,0,0,0,
                0,0,0,0,0,0,0,2,3,3,3,3,3,2,3,0,
                0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0 };

int cutcode;


/* main() : Wrapper for Thai LaTeX */
int main(argc, argv)
int argc;
char *argv[];
{
  FILE *fp, *fopen();
  unsigned char str[MAXLINELENGTH], out[MAXLINELENGTH];
  unsigned char *retval;
  int i, j, thaimode, c, cr;
  int testmode=0;
  
  cutcode = CUTCODE;
  
  fprintf(stderr,"C-TTeX $Revision: 1.1 $\n");
  fprintf(stderr,"Usage : cttex [cutcode] < infile > outfile\n");
  fprintf(stderr,"Usage : cutcode=0 forces operation in HTML mode.\n");
  fprintf(stderr,"Built-in dictionary size: %d words\n", numword);
  
  for(i=1;i<argc;i++) {
    if((argv[i][0] >= '0') && (argv[i][0] <= '9')) {
      sscanf(argv[i],"%d",&cutcode);
      if(cutcode) {		/* Test with given code */
	testmode = 1;
	fprintf(stderr, "Filter mode, cut code = %d\n", cutcode);
      }
      else {			/* HTML mode : use <WBR> code */
	cutcode = CUTCODE;
	testmode = 2;
	fprintf(stderr, "HTML mode\n");
      }
    }
  }
  
  i=0;
  fp = stdin;
  thaimode = cr = 0;
  while(!feof(fp)) {
    retval=fgets(str,MAXLINELENGTH-1,fp);
    if(!feof(fp)) {
      fixline(str);
      if(testmode) {               /* Non-TeX mode */
	if(testmode==1) {          /* Break with given code */
	  dooneline(str,out);
	  printf("%s",out);
	}
	else {                     /* Break with <WBR> tag */
	  dooneline(str,out);
	  j=0;
	  while(c=out[j]) {
	    if(c==cutcode) {
	      putchar('<');
	      putchar('W');
	      putchar('B');
	      putchar('R');
	      putchar('>');
	    }
	    else {
              if(HIGHWORD(c) && !thaimode) {
		putchar('<');
		putchar('N');
		putchar('O');
		putchar('B');
		putchar('R');
		putchar('>');
		thaimode = 1;
	      }
              if(!HIGHWORD(c) && thaimode) {
		putchar('<');
		putchar('/');
		putchar('N');
		putchar('O');
		putchar('B');
		putchar('R');
		putchar('>');
		thaimode = 0;
	      }
	      putchar(c);
            }
	    j++;
	  }
	}
      }
      else {                   /* TeX Mode */ 
	dooneline(str,out);
	adj(out);		/* Choose appropriate WANNAYUK */
	j = 0;
	while((c = (int)out[j])!=0) {
	  if(cr && thaimode) {
	    if(j!=0) {
	      fprintf(stderr, "\nLine %d doesn't end with NL\n", i+1);
	      fprintf(stderr, "%d found after NL\n", c);
	      fprintf(stderr, "BUG !! : Please report\n");
	      fprintf(stderr, "%sXXXXX\n",out);
	    }
	    if(HIGHWORD(c)) {
	      /* Add a % before newline in Thai Mode */
	      putchar('%');
	      putchar('\n');
	    }
	    else {
	      putchar('}');
	      putchar('\n');
	      thaimode = 0;
	    }
	    cr = 0;
	  }

	  /* Thai Mode */
	  if(thaimode) {              /* We got a CR in Thai mode */
	    if(c=='\n') {
	      cr = 1;		      /* Mark Flag */
	    }
	    else if(!HIGHWORD(c)) {   /* Leave ThaiMode */
	      putchar('}');
	      putchar(c);
	      thaimode = 0;
	    }
	    else {                    /* Remain in ThaiMode */
	      if(c==CUTCODE) 
		printf("\\tb ");
	      else {
		if(HIGHBIT)
		  putchar(c);
		else
		  printf("\\c%03d", c);
	      }
	    }
	  }

	  /* Not ThaiMode */
	  else {                      
	    if(!HIGHWORD(c))          /* Just print it out */
	      putchar(c);
	    else {                    /* A Thai Char detected */
	      if(c==CUTCODE) {        /* Just in case */
		fprintf(stderr, "\nCutCode found before Thai Characters\n");
		fprintf(stderr, "Line %d : BUG !! : Please report\n", i+1);
		printf("\\tb ");
	      }
	      else {
		if(HIGHBIT)
		  printf("{\\thai %c",c);
		else
		  printf("{\\thai\\c%03d",c);
	      }
	      thaimode = 1;
	    }
	  }
	  j++;
	}
      }
      i++;
      if(i%10==0)
        fprintf(stderr,"\r%4d",i);
    }
  } 
  fprintf(stderr,"\r%4d\n",i);
  if(cr && thaimode) {
    putchar('}');
    putchar('\n');
  }
  fprintf(stderr,"Done\n");

  return 0;
}

/* Word sep goes here */
void dooneline(unsigned char *in, unsigned char *out)
{
  int i,j,k,l,old_i;
  int wlist[MW], poslist[MW], jlist[MW], windex;
  int pos, fence, backmode;
  int prev_error=0;
    
  i=old_i=j=0;
  windex = 0;
  fence = 0;
  backmode = 0;
  prev_error = 0;
    
  while(in[i]) {

    /* old_i is the value of i before looking back mode */
    /* i>old_i means looking back was successful, cancel it */
    if(i>old_i)
      backmode=0;

    if(SKIPWORD(in[i])) {	        /* Chars to be skipped ? */
      if(prev_error) {
	/* Mark words not in dict */
	out[j++] = cutcode +1;
        prev_error=0;
      }
      backmode=fence=windex = 0;        /* Begin new word list */
      while(SKIPWORD(in[i]) && in[i]) {	/* Skip English char */
        out[j++] = in[i++];
      }
    }
    if(in[i])			        /* Still not EOL ? */
      do {
	if((k=findword(in+i,&pos))!=0) { /* Found in dict */
	  if(prev_error) {
	    /* Mark words not in dict */
	    out[j++] = cutcode + 1;
	    prev_error=0;
	  }
	  wlist[windex] = i;
	  poslist[windex] = pos;
	  jlist[windex] = j;
	  windex++;

	  /* For speed, limit the number of words to LOOK back */
	  /* by creating a fence */
	  /* Fence may only INCREASE */
	  if(windex - BACKDEPTH > fence) {
	    fence = windex - BACKDEPTH;
	  }
        
	  for(l=0;l<k;l++)	/* Copy word */
	    out[j++]=in[i++];

	  /* Mai Ya Mok & Pai Yan Noi */
	  /*	I was using this code before adding these two characters
		into the dict. Now they are in the dict and I no longer need
		these two lines.
		*/
	  while((in[i]==0xE6)||(in[i]==0xCF))
	    out[j++]=in[i++];

	  if(!SKIPWORD(in[i]))	/* Make sure it's not the last 
				   Thai word */
	    out[j++]=cutcode;	/* Insert word sep symbol */
	}
	else {			/* Not in Dict */

	  /* Shortening the prev wordS may help */
	  /* Try to Look Back */
	  while((windex > fence) && !k) {
	    /* Save status before looking back */
	    if(!backmode) {
	      backmode=1;
	      savestatus(&windex, wlist, poslist, jlist, &i, &j, out, 1);
	      old_i=i;
	    }
	    pos=poslist[windex-1]-1; /* Skip back one word */
	    while((pos>=0) && 
		  ((l=countmatch(wordptr[pos]+1,in+wlist[windex-1]))>0)) {
	      if((l==wordptr[pos][0]) && 
		 !NOTMIDDLE(in[wlist[windex-1]+l])) {
		k=1; break;
	      }
	      pos--;
	    }
	    /* A shorter version of prev word found */
	    if(k) {
	      out[j=jlist[windex-1]+l]=cutcode;
	      poslist[windex-1]=pos;
	      j++;
	      i=wlist[windex-1]+l;
	    }
	    else {
	      if(backmode && (windex==fence+1)) { 
		/* Search-Back method can't help, restore prev status */
		savestatus(&windex, wlist, poslist, jlist, &i, &j, out, 0);
		break;
	      }
	      windex--;
	    }
	  }
	  /* Sure that word is not in dictionary */
	  if(k==0) {
	    prev_error = 1;	/* Begin unknown word area */
	    out[j++]=in[i++];	/* Copy it */
	    backmode = fence=windex = 0; /* Clear Word List */
	  }
	}
      } while((k==0) && (!SKIPWORD(in[i])));
  }
  out[j]=0;

  /* Sth to do with words not in dict */
  /* (Remove 'cutcode+1') */
  filter(out);
}

/* Sequential verion */
/*
int findword(unsigned char *in)
{
  int i;

  for(i=numword-1;i>=0;i--) {
    if(mystrncmp(in,wordptr[i]+1,wordptr[i][0])==0) {
      printf("Found : %s %d\n", wordptr[i]+1,wordptr[i][0]);
      return wordptr[i][0];
    }
  }
  return 0;
}
*/

/* Calling : Index to a string
   Return  : Length of recognized word, and position of that word in
             dictionary
   Binary search method
*/

int findword(unsigned char *in, int *pos)
{
  int up,low,mid,a,l;

  up  = numword-1;              /* Upper bound */
  low = 0;                      /* Lower bound */
  *pos = -1;                    /* If word not found */
  
  /* Found word at the boundaries ? */
  if(mystrncmp(in,wordptr[up]+1,wordptr[up][0])==0) {
    *pos = up;
    return wordptr[up][0];
  }
  if(mystrncmp(in,wordptr[low]+1,wordptr[low][0])==0) {
    mid = low;
  }
  else {                        /* Begin Binary search */
  do {
    mid = (up+low)/2;
    a=mystrncmp(in,wordptr[mid]+1,wordptr[mid][0]);
/*
    printf("%d %d %d %s\n",low, mid, up,  wordptr[mid]+1);
*/
    if(a!=0) {
      if(a>0)
        low = mid;
      else
        up = mid;
    }
  } while((a!=0) && (up-low>1));
  if(a!=0) {                            /* Word not found */
    mid--;
    if(!countmatch(wordptr[mid]+1,in))  /* Can we find the shorter word ? */
      return 0;                         /* No, */

    while(mid && ((l=countmatch(wordptr[mid]+1,in))>0)) {
      if((l==wordptr[mid][0])&&!NOTMIDDLE(in[l])) {
        *pos = mid;
        return l;
      }
      mid--;
    }
    if(a) return 0;
  }
  }

  up = mid;
  if(up < numword)
  do {          /* Find the longest match */
    up++;
    a=mystrncmp(in,wordptr[up]+1,wordptr[up][0]);
    if(a==0) mid = up;
  } while((a>=0) && (up<numword-1));
/*
      printf("Found : %s %d\n", wordptr[mid]+1,wordptr[mid][0]);
*/
  *pos = mid;
  return wordptr[mid][0];
}

int countmatch(unsigned char *in, unsigned char *out)
{
  int i;

  i=0;
  while(in[i]==out[i])
    i++;
  return i;
}

void savestatus(int* windex, int* wlist, int* poslist, int* jlist,
                int* oi, int* j, unsigned char * out, int mode)
{
  static int lwindex, lwlist[MW], lposlist[MW], ljlist[MW], li, lj;
  int i;
  static unsigned char lout[MAXLINELENGTH];
    
/*
  printf("Save call %d\n",mode);
*/
  if(mode) {            /* Save */
    lwindex = *windex;
    for(i=0;i<lwindex;i++) {
      lwlist[i] = wlist[i];
      lposlist[i] = poslist[i];
      ljlist[i] = jlist[i];
    }
    for(i=0;i<*j;i++)
      lout[i]=out[i];
    li = *oi;
    lj = *j;
  }
  else {
    *windex = lwindex;
    for(i=0;i<lwindex;i++) {
      wlist[i] = lwlist[i];
      poslist[i] = lposlist[i];
      jlist[i] = ljlist[i];
    }
    for(i=0;i<lj;i++)
      out[i]=lout[i];
    *oi = li;
    *j = lj;
  }
}

/* Thai version of strncmp :
     b must be the word from dictionary
*/

int mystrncmp(a,b,l)
unsigned char *a, *b;
int l;
{
  int i;

/*   i=strncmp(a,b,l);
*/
  i=memcmp(a,b,l);
  if(i)
    return i;
  else {
    return(NOTMIDDLE(a[l]));
  }
}

/* What to do with words outside dictionary */
void filter(unsigned char *line)
{
  int i, j, c, a, found;
  unsigned char str[MAXLINELENGTH];

  strcpy(str,line);
  found=i=0;
  a=-1;
  while(c=str[i]) {
    if(c==cutcode) {
      a=i;
    }
    else if(c==cutcode+1) {
      found = 1;
      if(!SKIPWORD(str[i+1]))
	str[i] = cutcode;
      if(a>=0) {
	str[a] = cutcode+1;
	a=-1;
      }
    }
    else if(SKIPWORD(c)) {
      a=-1;
    }
    i++;
  }
  if(found) {
    i=j=0;
    while(c=str[i++]) 
      if(c!=cutcode+1)
	line[j++] = c;
    line[j]=0;
  }
}
	
/* Old one by Fong (Completely Removed)
   New one by Hui */
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
	    /* Special Case for Sara-Am */
	    if(c==0xD3) {
	      if(k>=0) {
		up[k]=0xED;
	      }
	      k++;
	      middle[k]=0xD2;      /* Put Sara-Ar */
	    }
	    else {
	      k++;
	      middle[k]=c; 
	    }
	    break;
	case 1 : /*Low*/
	    low[k]=c; break;
	case 2 : /*Up*/
	    up[k]=c; break;
	case 3 : /*Top*/
	    top[k]=c; break;
	}
    }

    /* Beauty Part Begins */

    for(i=0; i<=k; i++) {
      /* Move down from Top -> Up */
      if((top[i]) && (up[i]==0)) {
	up[i] = top[i] - 96;
	top[i] = 0;
      }

      /* Avoid characters with long tail */
      if( middle[i] == 0xBB ||           /* Por Pla */
	  middle[i] == 0xBD ||           /* For Far */
	  middle[i] == 0xBF ) {          /* For Fun */
	if(up[i]) 
	  up[i] = moveleft(up[i]);
	if(top[i]) 
	  top[i] = moveleft(top[i]);
      }

      /* Remove lower part of TorSanTan and YorPhuYing
	 if necessary */
      if(middle[i] == 0xB0 && low[i])    /* TorSanTan */
	middle[i] = 0x9F;
      if(middle[i] == 0xAD && low[i])    /* YorPhuYing */
	middle[i] = 0x90;

      /* Move lower sara down , for DorChaDa, TorPaTak */
      if(middle[i] == 0xAE ||
	 middle[i] == 0xAF ) {
	if(low[i]) 
	  low[i] = low[i] + 36;
      }
    }
    
    /* Pack Back To A Line */
    i=0; k=0;
    while(middle[i]){
	line[k++]=middle[i];
	if(low[i]) line[k++]=low[i];
	if(up[i])  line[k++]=up[i];
	if(top[i]) line[k++]=top[i];
	i++;
    }

    /* Numbef of Bytes might change */
    line[k]=0;
}

int lefttab[]={   136, 131,        /* Meaning : change 136 to 131, ... */
		  137, 132,        /* Up Level Mai Ek, To, Ti ... */
		  138, 133,
		  139, 134,
		  140, 135,
		 0xED, 0x8F,       /* Circle */
                 0xE8, 0x98,       /* Top Level Mai Ek, To, Ti, ... */
                 0xE9, 0x99,
                 0xEA, 0x9A,
                 0xEB, 0x9B,
                 0xEC, 0x9C,
                 0xD4, 0x94,       /* Sara I, EE, ... */
                 0xD5, 0x95, 
                 0xD6, 0x96, 
                 0xD7, 0x97, 
                 0xD1, 0x92,
                 0xE7, 0x93 };

int moveleft(int c)
{
  int i;

  for(i=0;i<34; i+=2) {
    if(lefttab[i] == c)
      return lefttab[i+1];
  }
  return c;
}

void fixline(line)
unsigned char *line;
{
  unsigned char top,up,middle,low;
  unsigned char out[MAXLINELENGTH];
  int i,j,c;

  i=j=0;
  strcpy(out,line);
  top=up=middle=low=0;
  while(c=out[i++]) {
    switch((c>0xD0)?levtable[c-0xD0]:0) {
    case 0 : 
      if(middle) {
	line[j++]=middle;
	if(low) line[j++]=low;
	if(up)  line[j++]=up;
	if(top) line[j++]=top;
      }
      top=up=middle=low=0;
      middle=c; break;
    case 1 : 
      low=c; break;
    case 2 : 
      up=c; break;
    case 3 : 
      top=c; break;
    }
  }
  if(middle) {
    line[j++]=middle;
    if(low) line[j++]=low;
    if(up)  line[j++]=up;
    if(top) line[j++]=top;
  }
  line[j]=0;
}


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.16  1996/09/01 13:34:49  vuthi
 * In HTML mode :
 *   Surround Thai Text with <NOBR> tags, to allow use of <WBR>
 *   in Microsoft Internet Explorer 3.0
 *   Without <NOBR>, <WBR> has no meaning in IE 3.0
 *   1.16b : Added fixline
 *
 * Revision 1.15  1995/10/06  13:09:52  vuthi
 * BUG FIXED : HTML mode worked only on the first line.
 *
 * Revision 1.14  1995/08/07  15:26:36  vuthi
 * HTML mode added
 *
 * Revision 1.13  1995/08/03  06:05:11  vuthi
 * Change "TEST MODE" to "FILTER MODE"
 *
 * Revision 1.12  1995/08/03  05:37:00  vuthi
 * Built-In dictionary (via .h file)
 * Perl script created
 * remove readdictfile()
 * remove -d option
 * dooneline() can be used alone (as a word-sep library).
 *
 * Revision 1.11  1995/08/03  04:47:22  vuthi
 * Fix bug in filter().. add if(SKIPWORD(c)) to reset 'a'
 *
 * Revision 1.10  1995/08/02  11:23:20  vuthi
 * Little bug fixed
 *
 * Revision 1.9  1995/08/02  11:19:21  vuthi
 * Add filter() to prevent word break before unknown words
 * Always break after unknown words
 *
 * Revision 1.8  1995/08/02  09:44:05  vuthi
 * New ADJ() algorithm.. Sara Am problem fixed.
 * moveleft() added.
 *
 * Revision 1.7  1995/07/22  17:43:50  vuthi
 * No breaking char at end of Thai word
 *
 * Revision 1.6  1995/04/25  12:11:28  vuthi
 * Use memcmp instead of strcmp to fix bug on some Japanized machine
 *
 * Revision 1.52  1995/4/24  23:26:00
 * 8-Bit version and use \tb instead of #254
 *
 * Revision 1.5  1994/12/23  08:45:06  vuthi
 * Bug of newline disappear at the end of Thai line
 *
 * Revision 1.4  1994/12/14  10:50:18  vuthi
 * Command Line Option, use "%" to terminal Thai lines
 *
 * Revision 1.3  1994/12/14  10:12:22  vuthi
 * Add Header Revision and Log
 *
 */

