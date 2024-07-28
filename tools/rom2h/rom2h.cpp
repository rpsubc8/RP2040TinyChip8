//Author: ackerman
//Convert CHIP8 roms to .h
//Tool for RP2040 chip8 emulator
//Max files: 255
//Max name file: 64 characters
//Max tile OSD emulator: 43 characters
//Characters support A..Z a..z 0..9
//Title: ( ) , - 0..9 : ; = A..Z [] _ a..z {}
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#define maxObjects 255
#define max_cad_title 43
#define max_cad_name_files 64
#define max_cad_tile_file 64
#define max_buffer_file_size 65536

unsigned char gb_bufferFile[max_buffer_file_size]; //64 KB archivo maximo

void listFilesRecursively(char *basePath);
void ShowHelp(void);
void WriteHeadROM_H(char *cadDefine);
void WriteFileROM(unsigned char num,char *cadPath, char *cadFile, char *cadFileSource);
void WriteSizeROM(void);
int GetSizeFile(char *cadFile);
void InitTitles(void);
void RemoveExt(char *cad);
void ProcesaFicherosROM(void);
//void TituloEspacios(char *cadOri, char *cadDest);
void AddOSDTitle(char *cadTitle,unsigned int id);

FILE *gb_fileWrite = NULL;
char gb_titles[maxObjects][(max_cad_tile_file+1)];
char gb_tilesOSD[maxObjects][(max_cad_tile_file+1)];
char gb_nameFiles[maxObjects][(max_cad_name_files+1)];
//char gb_nameDir[maxObjects][64];
unsigned char gb_contRom=0;


//**********************************************
void RemoveExt(char *cad)
{ 
 int total= strlen(cad);
 if (total > max_cad_title)
 {
  total= max_cad_title;
  cad[max_cad_title]='\0';
 }
 
 for (int i=0;i<total;i++)
 {
  //if (cad[i]==' ')
  if (cad[i]=='.')
  {
   cad[i]='\0';
   return;
  }
  else
  {
   //A..Z a..z 0..9
   if (!( ((cad[i]>=65)&&(cad[i]<=90)) || ((cad[i]>=97)&&(cad[i]<=122)) || ((cad[i]>=48)&&(cad[i]<=57)) ))
   { 
    cad[i] = '_';
   }
  }
 }
}

//**********************************************
void InitTitles()
{
 for (unsigned char i=0;i<maxObjects;i++)
 {
  gb_titles[i][0]='\0';
  gb_tilesOSD[i][0]='\0';
 }
}

//**********************************************
void ShowHelp()
{
 printf("Author: ackerman\n\n");
 printf("Convert rom to HEX .h emulator CHIP8 RP2040\n");
}

//**********************************************
int GetSizeFile(char *cadFile)
{
 long aReturn=0;
 FILE* fp = fopen(cadFile,"rb");
 if(fp) 
 {
  fseek(fp, 0 , SEEK_END);
  aReturn = ftell(fp);
  fseek(fp, 0 , SEEK_SET);// needed for next read from beginning of file
  fclose(fp);
 }
 return aReturn;
}

//**********************************************
void WriteFileROM(unsigned char num,char *cadPath,char *cadFile, char *cadFileSource)
{
 unsigned char contLine=0;
 FILE *auxWrite = NULL;
 FILE *auxRead = NULL;
 long auxSize=0;
 char cadDest[2048];
 char cadOri[2048];
 sprintf(cadOri,"%s\\%s",cadPath,cadFileSource);
 sprintf(cadDest,"output/dataFlash/roms/rom%s.h",cadFile);
 auxWrite = fopen(cadDest,"w+");
 if (auxWrite!= NULL)
 {
  fprintf(auxWrite,"#ifndef _ROM_%s_H\n",cadFile);
  fprintf(auxWrite," #define _ROM_%s_H\n",cadFile);     
  //WriteHexData(auxWrite,cadFile);  

  auxRead = fopen(cadOri,"rb");
  if (auxRead!=NULL)
  {
   auxSize = GetSizeFile(cadOri);
   
   fprintf(auxWrite," //rom %s %d bytes\n\n",cadFile,auxSize);
   fprintf(auxWrite,"const unsigned char gb_rom_%s[]={\n",cadFile);
      
   //printf ("Tam %d",auxSize);
   fread(gb_bufferFile,1,auxSize,auxRead);
   for (long i=0;i<auxSize;i++)
   {
    fprintf(auxWrite,"0x%02X",gb_bufferFile[i]);
    if (i<(auxSize-1))
     fprintf(auxWrite,",");
    contLine++;
    if (contLine>15)
    {
     contLine=0;
     fprintf(auxWrite,"\n");
    }
   }
   fclose(auxRead);
  }
  
  
  fprintf(auxWrite,"\n};\n");
  fprintf(auxWrite,"#endif\n");
  fclose(auxWrite);
 }
}

//**********************************************
//void TituloEspacios(char *cadOri, char *cadDest)
//{
// int topeOri= strlen(cadOri);
// for (int i=0;i<topeOri;i++)
// {
//  cadDest[i]= (cadOri[i]=='_')?' ':cadOri[i];
// }
// cadDest[topeOri]='\0';
// cadDest[max_cad_title]='\0';
//}

//**********************************************
void WriteHeadROM_H(char *cadDefine)
{//Los 48k
 //char cadOutTiles[max_cad_title];
 if (gb_fileWrite == NULL)
 {
  return;
 }
 fprintf(gb_fileWrite,"#ifndef %s\n",cadDefine);
 fprintf(gb_fileWrite," #define %s\n",cadDefine); 
 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," #include <stddef.h>\n");
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  if (strlen(gb_titles[i]) != 0)
  {
   fprintf(gb_fileWrite," #include \"roms/rom%s.h\"\n",gb_titles[i]);
  }
 }
 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," #define max_list_rom %d\n\n\n",gb_contRom);
 fprintf(gb_fileWrite," //roms\n //Titulos\n");
 fprintf(gb_fileWrite," static const char * gb_list_rom_title[max_list_rom]={\n");
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  if (strlen(gb_titles[i]) == 0)
  {
   fprintf(gb_fileWrite,"  \"\"");
  }
  else
  {
   //cadOutTiles[0]='\0';
   //TituloEspacios(gb_titles[i],cadOutTiles);
   //fprintf(gb_fileWrite,"  \"%s\"",cadOutTiles);
   fprintf(gb_fileWrite,"  \"%s\"",gb_tilesOSD[i]);
  }
  if (i<(gb_contRom-1))
  {
   fprintf(gb_fileWrite,",\n");
  }
 }
 fprintf(gb_fileWrite,"\n };\n");
 fprintf(gb_fileWrite,"\n");
 fprintf(gb_fileWrite," //Datos rom\n");
 fprintf(gb_fileWrite," static const unsigned char * gb_list_rom_data[max_list_rom]={\n");
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  if (strlen(gb_titles[i]) == 0)
  {
   fprintf(gb_fileWrite,"  NULL");
  }
  else 
  {
   fprintf(gb_fileWrite,"  gb_rom_%s",gb_titles[i]);
  }
  if (i<(gb_contRom-1))
  {
   fprintf(gb_fileWrite,",\n");   
  }
 }  
 fprintf(gb_fileWrite,"\n };\n");
 
 fprintf(gb_fileWrite,"\n");
 //fprintf(gb_fileWrite,"#endif\n");
}


//********************************************
void WriteSizeROM()
{
 char cadDestino[1024];
 int auxSize;
 if (gb_fileWrite == NULL)
 {
  return;
 }
 fprintf(gb_fileWrite," //Tamanio en bytes\n");
 fprintf(gb_fileWrite," static const unsigned short int gb_list_rom_size[max_list_rom]={\n");
 for (unsigned char i=0;i<gb_contRom;i++)
 {
  if (strlen(gb_titles[i]) == 0)
  {
   fprintf(gb_fileWrite,"  \"\"");
  }
  else
  {
   sprintf(cadDestino,"input\\roms\\%s",gb_nameFiles[i]);   
   auxSize = GetSizeFile(cadDestino);
   fprintf(gb_fileWrite,"  %d",auxSize);
  }
  if (i<(gb_contRom-1))
  {
   fprintf(gb_fileWrite,",\n");
  }
 }
 fprintf(gb_fileWrite,"\n };\n");
 fprintf(gb_fileWrite,"\n");  
}

//**********************************************
void AddOSDTitle(char *cadTitle,unsigned int id)
{
 int total= strlen(cadTitle);
 if (total > max_cad_title)
 {
  total= max_cad_title;
  gb_tilesOSD[id][max_cad_title]='\0';
 }
 
 for (int i=0;i<total;i++)
 {
  if (cadTitle[i]=='.')
  {
   gb_tilesOSD[id][i]='\0';
   return;
  }
  else
  {
   // ( ) , - 0..9 : ; = A..Z [] _ a..z {}
   if (
       (cadTitle[i]==32)
       || ((cadTitle[i]>=40)&&(cadTitle[i]<=41))
       || (cadTitle[i]==44)
       || (cadTitle[i]==45)
       || ((cadTitle[i]>=48)&&(cadTitle[i]<=57))
       || (cadTitle[i]==58)
       || (cadTitle[i]==59)
       || (cadTitle[i]==61)
       || ((cadTitle[i]>=65)&&(cadTitle[i]<=90))
       || (cadTitle[i]==91)
       || (cadTitle[i]==93)
       || (cadTitle[i]==95)
       || ((cadTitle[i]>=97)&&(cadTitle[i]<=122))
       || (cadTitle[i]==123)
       || (cadTitle[i]==125)
      )
   {
    gb_tilesOSD[id][i]= cadTitle[i];
   }
   else
   {    
    gb_tilesOSD[id][i]= '_';
   }
  }
 }     
}

//**********************************************
void listFilesRecursively(char *basePath)
{     
    char cadFileSource[1000];
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir)
    {
     return; // Unable to open directory stream
    }
    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            printf("%s\n", dp->d_name);
            
            strcpy(cadFileSource,dp->d_name);
            strcpy(gb_nameFiles[gb_contRom],dp->d_name); //Nombre completo
            AddOSDTitle(dp->d_name,gb_contRom); //OSD quito algunos caracteres y aniado en listad osd
            RemoveExt(dp->d_name); //Quito caracteres raros para C includoe
            strcpy(gb_titles[gb_contRom],dp->d_name);

            WriteFileROM(gb_contRom,basePath,dp->d_name,cadFileSource);            
                        
            gb_contRom++;
            if (gb_contRom > (maxObjects-1))
            {
             return;
            }
            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            listFilesRecursively(path);
        }
    }
    closedir(dir);
}

//*************************************************
void ProcesaFicherosROM()
{
 gb_contRom=0;
 gb_fileWrite = fopen("output/dataFlash/gbrom.h","w+");
 if (gb_fileWrite!= NULL)
 {
  listFilesRecursively("input/roms");
  WriteHeadROM_H("_GB_ROM_H");
  WriteSizeROM();
  fprintf(gb_fileWrite,"#endif\n");
  fclose(gb_fileWrite);
 }     
}


//********
//* MAIN *
//********
int main(int argc, char**argv)
{
 char path[100];    //Directory path to list files
 ShowHelp();

 InitTitles();
 
 ProcesaFicherosROM();
 //printf("Enter path to list files: ");    // Input path from user
 //scanf("%s", path);
 //listFilesRecursively(path);
 
 return 0;
}
