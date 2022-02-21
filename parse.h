enum state {
    START,
    STREAM,
    DOCUMENT,
    SECTION,

    CLIST,	// new
    CVALUES,// new
    CKEY,	// new
	CCHECK, // new
	QLIST,	// new
	QVALUES,// new
	QKEY,	// new
    IP,
    PORT,
    PROTOCOLID,
    SERVERID,
    FC,
    STARTREGADDR,
    COMMAND,
    STOP
};

typedef struct query_t{
    char *ip;
    int port;
    int protocolId;
    int serverId;
    int fc;
    int startRegAddr;
    int command;
	struct query_t *next;
} query_t;

typedef struct parse_state_t{
    int isclient;	//0 no, 1 yes
	char *addr;
    enum state state;
	struct query_t q;
	struct query_t *qlist;
	int count;
} parse_state_t;

char *bail_strdup(const char *s);
void add_query(query_t **queries, char *ip, int port, int protocolId, int serverId, int fc, int startRegAddr, int command);
int consume_event(parse_state_t *s, yaml_event_t *e);
