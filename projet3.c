//
//  projet3.c
//
//
//  Created by Cao Sang DOAN on 24/03/15.
//
//

#include "projet3.h"

int min_dimension;

unsigned long hex2int(char *a, unsigned int len){
    int i, tmp;
    
    unsigned long val = 0;
    for (i = 0; i < len; i++) {
        switch (a[i]) {
            case 'a':
                val = val + (10<<((len - 1 - i)*4));
                break;
            case 'b':
                val = val + (11<<((len - 1 - i)*4));
                break;
            case 'c':
                val = val + (12<<((len - 1 - i)*4));
                break;
            case 'd':
                val = val + (13<<((len - 1 - i)*4));
                break;
            case 'e':
                val = val + (14<<((len - 1 - i)*4));
                break;
            case 'f':
                val = val + (15<<((len - 1 - i)*4));
                break;
            default:
                if (isdigit(a[i]) == 0) {
                    printf("hex2int: erreur not a number %c %d!\n", a[i], i);
                    exit(1);
                }
                tmp = a[i] - '0';
                val = val + (tmp<<((len - 1 - i)*4));
                break;
        }
    }
    
    return val;
}

char *create_filter(char *keyword, int size_filter){
    char *key, *tmp, *tmp2;
    char *filter = malloc(sizeof(char) *(size_filter + 1));
    unsigned char md[32];
    char hash[2*sizeof(md) + 1];
    CC_SHA256_CTX c;
    int i;
    unsigned long l;
    
    for (i = 0; i < size_filter; i++) {
        filter[i] = '0';
    }
    filter[i] = '\0';

    tmp = keyword;
    key = strtok(tmp, ",");
    
    while (key != NULL) {
        CC_SHA256_Init(&c);
        CC_SHA256_Update(&c, (const void *)key, strlen(key));
        CC_SHA256_Final(md, &c);
       
        for (i = 0; i < sizeof(md); i++) {
            snprintf(hash+(2*i), 3, "%02x\n", (int)md[i]);
        }
       
        l = hex2int(hash, 32);
        filter[l % size_filter] = '1';
        
        key = strtok(NULL, ",");
    }

    filter[strlen(filter)] = '\0';

    return filter;
}

int generator_filter(char *name){
    FILE *file_in, *file_out;

    file_in = fopen(name, "r");
    if (file_in == NULL) {
        printf("generator_filter: error open file_in %s/n", name);
        exit(1);
    }
    
    char tmp[1<<20];
    char *docUrl, *description, *filter;
    int nbreq, nbKeywords, pageRank;
    
    if (fgets(tmp, 1<<20, file_in) == NULL){
        printf("generator_filter: file %s empty\n", name);
        exit(1);
    }

    CC_SHA1_CTX c;
    unsigned char md[20];
    char hash[2*sizeof(md) + 1];
    int i, j = 0;
    char file_name[128];
    char *f = "filter/";

    while (fgets(tmp, 1<<20, file_in) != NULL) {
        docUrl = strtok(tmp, ";");
        description = strtok(NULL, ";");
        pageRank = atoi(strtok(NULL, ";"));
        nbreq = atoi(strtok(NULL, ";"));
        nbKeywords = atoi(strtok(NULL, ";"));

        filter = create_filter(description, 512);

        CC_SHA1_Init(&c);
        CC_SHA1_Update(&c, (const void *)filter, strlen(filter));
        CC_SHA1_Final(md, &c);
        
        for (i = 0; i < sizeof(md); i++) {
            snprintf(hash+(2*i), 3, "%02x\n", (int)md[i]);
        }

        strcpy(file_name, DIR);
        strcat(file_name, f);
        strcat(file_name, hash);
        file_name[strlen(file_name)] = '\0';

        file_out = fopen(file_name, "w+");

        fprintf(file_out, "%s\n", docUrl);
        fclose(file_out);
    
        put(filter);
        j++;
    }
    
    printf("taille bench %d\n", j);
    fclose(file_in);
    return 0;
}

char *create_vector(char *filtre,int n){
    if (n > 6) {
        printf("create_vector: n > 6 \n");
        return NULL;
    }

    if ((strlen(filtre) > MAX_FILTER) || (strlen(filtre) < MIN_FILTER)) {
        printf("create_vector: filtre > 512 or filter < 16\n");
        return NULL;
    }
    
    char *vector = (char *)malloc(sizeof(char)*257);
    char *tmp = filtre;
    int i, j = 0;

    for (i = 0; i < MAX_FILTER; i = i + 8*pow(2, MAX_DIMENSION - n)){
        strncpy(vector+j, tmp+i, 4);
        j = j+4;
    }

    vector[j] = '\0';
    return vector;
}

int create_file_vector(char *vector, char *data){
    FILE *f;
    char *file_vector;
    
    char tmp[1024], *tmp2;
    int index = -1, i;

    file_vector = find_file_name_vector(vector);
    printf("file_vector : -%s-\n", file_vector);

    f = fopen(file_vector, "r+");

    if (f != NULL) {

        while (fgets(tmp, 1024, f) != NULL) {
            tmp[strlen(tmp) - 1] = '\0';
            
            index = strtol(strtok(tmp, ";"), NULL, 10);
            tmp2 = strdup(strtok(NULL, ";"));
            if (strcasecmp(tmp2, data) == 0) {
         //       printf("create_file_vector: existe déjà %s\n", data);
                break;
            }
        }

        if (strcasecmp(tmp2, data) == 0) {
            fclose(f);
            free(file_vector);
            return 0;
        }
    }else{
        f = fopen(file_vector, "a+");
        
        if (f == NULL) {
            printf("create_file_vector: error create %s\n", file_vector);
            free(file_vector);
            exit(1);
        }
    }
    fprintf(f, "%d;%s\n", index + 1, data);
    fflush(f);
    fclose(f);
 
    free(file_vector);
    return 0;
}

char *find_file_name_vector(char *vector){
    char *file_vector = (char *)malloc(sizeof(char) *65);
    memset((void *)file_vector, '\0', sizeof(char)*strlen(file_vector));

    CC_SHA1_CTX c;
    unsigned char md[20];
    char hash[2*sizeof(md) + 1];
    char tmp[5];
    int i;
    
    CC_SHA1_Init(&c);
    CC_SHA1_Update(&c, (const void *)vector, strlen(vector));
    CC_SHA1_Final(md, &c);
    
    for (i = 0; i < sizeof(md); i++) {
        snprintf(hash+(2*i), 3, "%02x\n", (int)md[i]);
    }
    
    memset((void *)tmp, '\0', sizeof(char)*strlen(tmp));

    switch (strlen(vector)) {
        case 256:
            sprintf(tmp, "%d/", 6);
            break;
            
        case 128:
            sprintf(tmp, "%d/", 5);
            break;
            
        case 64:
            sprintf(tmp, "%d/", 4);
            break;
            
        case 32:
            sprintf(tmp, "%d/", 3);
            break;
            
        case 16:
            sprintf(tmp, "%d/", 2);
            break;
         
        case 8:
            sprintf(tmp, "%d/", 1);
            break;
            
        default:
            sprintf(tmp, "%d/", 0);
            break;
    }

    strncpy(file_vector, DIR, strlen(DIR));
    strncpy(file_vector + (strlen(DIR)), tmp, 2 );
    strncpy(file_vector + (strlen(DIR) + 2), VEC, strlen(VEC));
    strcpy(file_vector + (strlen(DIR) + strlen(VEC) + 2), hash);
    file_vector[strlen(file_vector)] = '\0';

    return file_vector;
}

unsigned long bit2int(char *a, unsigned int len){
    int i;
    unsigned long val = 0;
    
    if (strlen(a) < len) {
        printf("bit2int: strlen(a) < len\n");
        exit(1);
    }
    for (i = 0; i < len; i++) {
        switch (a[i]) {
            case '1':
                if (isdigit(a[i]) == 0) {
                    printf("bit2int: erreur not a number %c!\n", a[i]);
                    exit(1);
                }
                val = val + (1<<((len - 1 - i)));
                break;
            default:
                if (isdigit(a[i]) == 0) {
                    printf("bit2int: erreur not a number %c!\n", a[i]);
                    exit(1);
                }
                break;
        }
    }
    return val;
}

char *int2bit(int a, int len){
    int i;
    int tmp = a;
    
    if ((a > (pow(2, len) - 1)) || (a < 0)) {
        printf("int2bit: erreur a et len\n");
        exit(1);
    }
    
    char *val = (char *)malloc(sizeof(char));
    for (i = 0; i < len; i++) {
        if ((1<<(len - i - 1)) <= tmp) {
            val[i] = '1';
            tmp = tmp - (1<<(len - i - 1));
        }else{
            val[i] = '0';
        }
    }
    val[i] = '\0';
    return val;
}

int put(char *filtre){
    FILE *f, *v;

    char file_name[128];
    
    strcpy(file_name, DIR);
    strcpy(file_name+2, VA_FILE);
    file_name[strlen(file_name)] = '\0';
    
    f = fopen(file_name, "r+");
    if (f == NULL) {
        f = fopen(file_name, "w+");
        if (f == NULL) {
            printf("PUT: error 1 create %s\n", file_name);
            exit(1);
        }
    }

    char *vector_tmp = create_vector(filtre, min_dimension);
    int i;
    
    char tmp[1<<20], tmp3[1<<20];
    int index = -1;
    char *tmp2, *tmp4;

    index = -1;
    tmp2 = "";
    tmp4 = "";
    
    i = 0;
    
    int j;
    
    while (fgets(tmp, 1<<20, f) != NULL) {
        tmp[strlen(tmp) - 1] = '\0';

        index = strtol(strtok(tmp, ";"), NULL, 10);
        tmp2 = strtok(NULL, ";");
        
        if (strcasecmp(tmp2, vector_tmp) == 0) {
            //printf("PUT: vecteur existe déjà %s\n", vector_tmp);
            break;
        }
        memset((void *)tmp, '\0', sizeof(char)*strlen(tmp));

    }
    
    if (strcasecmp(tmp2, vector_tmp) != 0) {
        fprintf(f, "%d;%s\n", index + 1, vector_tmp);
        fflush(f);
    }
    
    for (i = min_dimension; i <= MAX_DIMENSION; i++) {
        if (i == MAX_DIMENSION) {
            vector_tmp = create_vector(filtre, i);
            create_file_vector(vector_tmp, filtre);
        }else{
            vector_tmp = create_vector(filtre, i);
            create_file_vector(vector_tmp, create_vector(filtre, i + 1));
        }
    }
   
    fclose(f);
   
    return 0;
}

int search(char *filtre){

    FILE *f, *v;

    char file_name[128];
    
    strcpy(file_name, DIR);
    strcpy(file_name+2, VA_FILE);
    file_name[strlen(file_name)] = '\0';
    
    f = fopen(file_name, "r");
  
    if (f == NULL) {
        printf("SEARCH: error 1 fopen %s\n", file_name);
        exit(1);
    }
  

    char vector_tmp[512];
    strcpy(vector_tmp , create_vector(filtre, min_dimension));
    
    char res[1<<20];
    int i;
  
    char tmp[1<<20], *tmp2, *tmp3, *tmp4;
    char tmp22[1<<20], tmp33[1<<20], tmp44[1<<20], tmp444[1<<20];

    char *file_vector;
    int index;
    i = 0;
    
    memset((void *)res, '\0', sizeof(char)*strlen(res));

    while (fgets(tmp, 1<<20, f) != NULL) {
        tmp[strlen(tmp) - 1] = '\0';
        
        index = strtol(strtok(tmp, ";"), NULL, 10);
        tmp2 = strtok(NULL, ";");
        
        if (ainb(vector_tmp, tmp2)) {
            sprintf(tmp22, "%s;", tmp2);
            strcat(res, tmp22);
        }
        printf("max_size = %lu %lu\n", strlen(res), sizeof(res));

    }
    
    fclose(f);
    
    res[strlen(res) - 1] = '\0';
    
    if (strlen(res) == 0) {
        printf("SEARCH: not found\n");
        return 1;
    }

    printf("res trouve %s\n", res);
    
    char *r;
    for (i = (min_dimension + 1); i <= MAX_DIMENSION; i++) {
        
        r = strdup(res);
        tmp2 = strsep(&r, ";");

        memset((void *)vector_tmp, '\0', sizeof(char)*strlen(vector_tmp));
        memset((void *)tmp44, '\0', sizeof(char)*strlen(tmp44));
        memset((void *)tmp33, '\0', sizeof(char)*strlen(tmp33));

        strcpy(vector_tmp , create_vector(filtre, i));
        
        while (tmp2 != NULL) {
            
            file_vector = find_file_name_vector(tmp2);
            
            f = fopen(file_vector, "r");
            
            if (f == NULL) {
                break;
            }
            
            memset((void *)tmp, '\0', sizeof(char)*strlen(tmp));

            index = -1;
            while (fgets(tmp, 1<<20, f) != NULL) {
                tmp[strlen(tmp) - 1] = '\0';
                
                index = strtol(strtok(tmp, ";"), NULL, 10);
                tmp3 = strtok(NULL, ";");
                
                if (ainb(vector_tmp, tmp3)) {

                    sprintf(tmp33, "%s;", tmp3);
                    if (strlen(tmp44) >= 1048515) {
                        strcat(tmp444, tmp33);
                    }else{
                        strcat(tmp44, tmp33);
                    }
                }
                printf("max_size = %lu %lu\n", strlen(tmp44), sizeof(tmp44));

            }
           
            fclose(f);
            tmp2 = strsep(&r, ";");
        }
        
       
        tmp44[strlen(tmp44) - 1] = '\0';

        memset((void *)res, '\0', sizeof(char)*strlen(res));
        strcpy(res, tmp44);
    }
    
    
    r = strdup(res);
    tmp2 = strsep(&r, ";");
        
    memset((void *)tmp44, '\0', sizeof(char)*strlen(tmp44));
    memset((void *)tmp33, '\0', sizeof(char)*strlen(tmp33));
    
    while (tmp2 != NULL) {
        
        file_vector = find_file_name_vector(tmp2);
        
        f = fopen(file_vector, "r");
        
        if (f == NULL) {
            break;
        }
        
        memset((void *)tmp, '\0', sizeof(char)*strlen(tmp));
        
        index = -1;
        while (fgets(tmp, 1<<20, f) != NULL) {
            tmp[strlen(tmp) - 1] = '\0';
            
            index = strtol(strtok(tmp, ";"), NULL, 10);
            tmp3 = strtok(NULL, ";");
            
            if (ainb(filtre, tmp3)) {
                sprintf(tmp33, "%s;", tmp3);
                strcat(tmp44, tmp33);
            }
            
        }
        
        fclose(f);

        tmp2 = strsep(&r, ";");
    }
    
    tmp44[strlen(tmp44) - 1] = '\0';
    
    memset((void *)res, '\0', sizeof(char)*strlen(res));
    strcpy(res, tmp44);
    
    CC_SHA1_CTX c;
    unsigned char md[20];
    char hash[2*sizeof(md) + 1];
    char *t = "filter/";
    char *resultat = (char *)malloc(sizeof(char) * 1<<20);

    tmp2 = strtok(res, ";");
    
    while (tmp2 != NULL) {
        memset((void *)file_name, '\0', sizeof(char)*strlen(file_name));

        CC_SHA1_Init(&c);
        CC_SHA1_Update(&c, (const void *)tmp2, strlen(tmp2));
        CC_SHA1_Final(md, &c);
        
        for (i = 0; i < sizeof(md); i++) {
            snprintf(hash+(2*i), 3, "%02x\n", (int)md[i]);
        }
        
        strcpy(file_name, DIR);
        strcat(file_name, t);
        strcat(file_name, hash);
        file_name[strlen(file_name)] = '\0';
        
        f = fopen(file_name, "r");
        
        memset((void *)tmp, '\0', sizeof(char)*strlen(tmp));

        while (fgets(tmp, 1<<20, f) != NULL) {
            tmp[strlen(tmp) - 1] = '\0';
/////////////////////////
        }
        fclose(f);
     
        tmp2 = strtok(NULL, ";");
    }
    
    return 0;
 
}

int generator_request(char *name){
    FILE *file_in;
    
    file_in = fopen(name, "r");
    if (file_in == NULL) {
        printf("generator_request: error open file_in %s/n", name);
        exit(1);
    }
    
    char tmp[1<<20];
    char *docUrl, *description, *filter;
    int nbreq, nbKeywords, pageRank;
    
    if (fgets(tmp, 1<<20, file_in) == NULL){
        printf("generator_filter: file %s empty\n", name);
        exit(1);
    }
    
    int i, j = 0;
    
    while (fgets(tmp, 1<<20, file_in) != NULL) {
        docUrl = strtok(tmp, ";");
        description = strtok(NULL, ";");
        pageRank = atoi(strtok(NULL, ";"));
        nbreq = atoi(strtok(NULL, ";"));
        nbKeywords = atoi(strtok(NULL, ";"));
        
        filter = create_filter(description, 512);
        printf("filter: -%s-\n", filter);
        search(filter);
        j++;
    }
    
    printf("taille bench %d\n", j);

    fclose(file_in);
    return 0;
}

int ainb(char *a, char *b){
    if (strlen(a) != strlen(b)) {
        printf("size of != size of b\n");
        return 0;
    }
    
    int i;
    
    for (i = 0; i < strlen(a); i++) {
        if (*(a+i) == '1') {
            if (*(b+i) == '1') {
                continue;
            }else{
                return 0;
            }
        }
    }
    return 1;
}

int main(int argc, char **argv){
    min_dimension = 1;
  //  generator_filter(argv[1]);
    
    generator_request(argv[1]);
    
    search("00000000000000000001000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000100000001000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000100000000000000000000000100000000000000001000000000000000000000000001000000000000000000000000000000000000000000000000000000000100000000000000000000000100000010000000000000000000010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    
    return 0;
}






