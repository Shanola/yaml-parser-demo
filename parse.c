#include <stdio.h>
#include <stdint.h>

#include <yaml.h>
#include "parse.h"

char *bail_strdup(const char *s)
{
    char *c = strdup(s ? s : "");
    if (!c) {
        printf("out of memory");
    }
    return c;
}

void add_query(query_t **queries, char *ip, int port, int protocolId, int serverId, int fc, int startRegAddr, int command)
{
    query_t *q = calloc(1, sizeof(*q));
	if (!q) {
	    printf("calloc failed\n");
	}
	q->ip = bail_strdup(ip);
	q->port = port;
	q->protocolId = protocolId;
	q->serverId = serverId;
	q->fc = fc;
	q->startRegAddr = startRegAddr;
	q->command = command;

	if (!*queries) {
	    *queries = q;
	} else {
	    query_t *tmp = *queries;
		while (tmp->next) {
		    tmp = tmp->next;
		}
		tmp->next = q;
	}
}

int consume_event(parse_state_t *s, yaml_event_t *e)
{
    char *value;
    int event = e->type;
    switch(s->state) {
	case START:
	    switch (event) {
		case YAML_STREAM_START_EVENT:
		    s->state = STREAM;
			break;
		default:
		    printf("0unexpected event %d in state %d\n", e->type, s->state);
			return 0;
		}
		break;
	case STREAM:
	    switch (event) {
		case YAML_DOCUMENT_START_EVENT:
		    s->state = DOCUMENT;
			break;
		case YAML_STREAM_END_EVENT:
		    s->state = STOP;
			break;
		default:
	        printf("1unexpected event %d in state %d\n", e->type, s->state);
		    return 0;
		}
		break;
	case DOCUMENT:
	    switch (event) {
		case YAML_MAPPING_START_EVENT:
		    s->state = SECTION;
			break;
		case YAML_DOCUMENT_END_EVENT:
		    s->state = STREAM;
			break;
		default:
		    printf("2unexpected event %d in state %d\n", e->type, s->state);
			return 0;
		}
		break;
	case SECTION:
	    switch(event) {
		case YAML_SCALAR_EVENT:
		    value = (char *)e->data.scalar.value;
			if (strcmp(value, "client") == 0) {
			    s->state = CLIST;
			} else {
			    return 0;
			}
		    break;   
		case YAML_DOCUMENT_END_EVENT:
		    s->state = STREAM;
			break;
		default:
		    printf("3unexpected event %d in state %d\n", e->type, s->state);
			return 0;
		}
		break;
	case CLIST:
	    switch(event) {
		case YAML_SEQUENCE_START_EVENT:
		    s->state = CVALUES;
		    break;
		case YAML_MAPPING_END_EVENT:
		    s->state = SECTION;
		    break;
		default:
		    printf("4unexpected event %d in state %d\n", e->type, s->state);
			return 0;
		}
	    break;
	case CVALUES:
	    switch (event) {
		case YAML_MAPPING_START_EVENT:
		    s->state = CKEY;
			break;
		case YAML_SEQUENCE_END_EVENT:
		    s->state = CLIST;
			break;
		default:
		    printf("5unexpected event %d in state %d\n", e->type, s->state);
			return 0;
		}
	    break;
	case CKEY:
	    switch (event) {
	    case YAML_SCALAR_EVENT:
		    value = (char *)e->data.scalar.value;
			if (strcmp(value, "clientIP") == 0) {
			    s->state = CCHECK;
			} else if (strcmp(value, "query") == 0) {
			    s->state = QLIST;
			} else {
			    printf("unexpected key: %s\n", value);
				return 0;
			}
			break;
	    case YAML_MAPPING_END_EVENT:
		    /*if (isclient) {
		        add_query(&s->qlist, s->q.ip, s->q.port, s->q.protocolId, s->q.serverId, s->q.fc, s->q.startRegAddr, s->q.command);
			}*/
			s->isclient = 0; // reset 
			s->state = CVALUES;
	        break;
		default:
		    printf("6unexpected event %d in state %d\n", e->type, s->state);
			return 0;
	    }
		break;
	case CCHECK:
	    switch (event) {
	    case YAML_SCALAR_EVENT:
	        value = (char *)e->data.scalar.value;
		    if (strcmp(value, s->addr) == 0) {
		        printf("Got client's addr: %s\n", value);
			    s->isclient = 1;
		    }
		    s->state = CKEY;
	        break;
	    default:
	        printf("7unexpected event %d in state %d\n", e->type, s->state);
		    return 0;
	    }
	    break;
	case QLIST:
	    switch (event) {
		case YAML_SEQUENCE_START_EVENT:
		    s->state = QVALUES;
			break;
		default:
	        printf("8unexpected event %d in state %d\n", e->type, s->state);
		    return 0;		    
		}
		break;
	case QVALUES:
	    switch (event) {
		case YAML_MAPPING_START_EVENT:
		    s->state = QKEY;
		    break;
		case YAML_SEQUENCE_END_EVENT:
		    s->state = CKEY;
		    break;
		default:
	        printf("9unexpected event %d in state %d\n", e->type, s->state);
		    return 0;		    
		}
	    break;
	case QKEY:
	    switch (event) {
		case YAML_SCALAR_EVENT:
		    value = (char *)e->data.scalar.value;
			if (strcmp(value, "ip") == 0) {
			    s->state = IP;
			} else if(strcmp(value, "port") == 0) {
			   s->state = PORT; 
			} else if (strcmp(value, "protocolId") == 0) {
			    s->state = PROTOCOLID;
			} else if (strcmp(value, "serverId") == 0) {
			    s->state = SERVERID;
			} else if (strcmp(value, "fc") == 0) {
			    s->state = FC;
			} else if (strcmp(value, "startRegAddr") == 0) {
			    s->state = STARTREGADDR;
			} else if (strcmp(value, "command") == 0) {
			    s->state = COMMAND;
			} else {
		        printf("unexpected key: %s\n", value);
			    return 0;
			}
		    break;
		case YAML_MAPPING_END_EVENT:
		    if (s->isclient) {
		        add_query(&s->qlist, s->q.ip, s->q.port, s->q.protocolId, s->q.serverId, s->q.fc, s->q.startRegAddr, s->q.command);
			}
			s->state = QVALUES;
		    break;
		default:
	        printf("10unexpected event %d in state %d\n", e->type, s->state);
		    return 0;		    
		}
	    break;
	case IP:
	    switch (event) {
		case YAML_SCALAR_EVENT:
		    if (s->q.ip) {
			    printf("duplicate key: ip\n");
				free(s->q.ip);
			}
			s->q.ip = bail_strdup((char *)e->data.scalar.value);
			if (!s->q.ip) {
			    printf("strdup failed\n");
				return 0;
			}
			s->state = QKEY;
			break;
		default:
		    printf("11unexpected event %d in state %d\n", e->type, s->state);
			return 0;		    
		}
	    break;
	case PORT:
	    switch (event) {
		case YAML_SCALAR_EVENT:
		    if (s->q.port) {
			    printf("duplicate key: port\n");
			}
			s->q.port = atoi((char *)e->data.scalar.value);
			s->state = QKEY;
			break;
		default:
		    printf("12unexpected event %d in state %d\n", e->type, s->state);
			return 0;		    
		}
	    break;
	case PROTOCOLID:
	    switch (event) {
		case YAML_SCALAR_EVENT:
		    if (s->q.protocolId) {
			    printf("duplicate key: protocolId\n");
			}
			s->q.protocolId = atoi((char *)e->data.scalar.value);
			s->state = QKEY;
			break;
		default:
		    printf("13unexpected event %d in state %d\n", e->type, s->state);
			return 0;		    
		}
	    break;
	case SERVERID:
	    switch (event) {
		case YAML_SCALAR_EVENT:
		    if (s->q.serverId) {
			    printf("duplicate key: serverId\n");
			}
			s->q.serverId = atoi((char *)e->data.scalar.value);
			s->state = QKEY;
			break;
		default:
		    printf("14unexpected event %d in state %d\n", e->type, s->state);
			return 0;		    
		}
	    break;
	case FC:
	    switch (event) {
		case YAML_SCALAR_EVENT:
		    if (s->q.fc) {
			    printf("duplicate key: fc\n");
			}
			s->q.fc = atoi((char *)e->data.scalar.value);
			s->state = QKEY;
			break;
		default:
		    printf("15unexpected event %d in state %d\n", e->type, s->state);
			return 0;		    
		}
	    break;
	case STARTREGADDR:
	    switch (event) {
		case YAML_SCALAR_EVENT:
		    if (s->q.startRegAddr) {
			    printf("duplicate key: startRegadd\n");
			}
			s->q.startRegAddr = atoi((char *)e->data.scalar.value);
			s->state = QKEY;
			break;
		default:
		    printf("16unexpected event %d in state %d\n", e->type, s->state);
			return 0;		    
		}
	    break;
	case COMMAND:
	    switch (event) {
		case YAML_SCALAR_EVENT:
		    if (s->q.command) {
			    printf("duplicate key: command\n");
			}
			s->q.command = atoi((char *)e->data.scalar.value);
			s->state = QKEY;
			break;
		default:
		    printf("17unexpected event %d in state %d\n", e->type, s->state);
			return 0;		    
		}
	    break;
	case STOP:
	    break;
	}
	return 1;
}
