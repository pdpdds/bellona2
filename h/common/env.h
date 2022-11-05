#ifndef ENV_HEADER_jj
#define ENV_HEADER_jj

typedef struct {
    int         nBuffSize;      // 환경 변수 버퍼 크기.
    int         nDataSize;      // 데이터가 저장된 크기.
    char        *pBuff;         // 환경 변수 버퍼.
} UEnvStt;

extern int init_env_buff( UEnvStt *pEnv );
extern int close_env_buff( UEnvStt *pEnv );
extern int get_env_str( char *pName, char *pStr );

extern BELL_EXPORT void disp_env_str();
extern BELL_EXPORT int set_env_str( char *pName, char *pStr );

#endif