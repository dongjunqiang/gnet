#define GNET_NAME_MAX 32
#define GNET_MSG_MAX (32 << 10)

/*
int gnet_create(const char* name, const char* master_host, int16_t master_port)
{
    Reactor* reactor = new Reactor;
    
    Connector* con = new Connector(reactor, fd);
}


typedef struct gnet_msg {
    size_t size;
    char data[GNET_MSG_MAX];
    char src[GNET_NAME_MAX];
    char dst[GNET_NAME_MAX];
} gmsg;

int gnet_create(const char* name, const char* master_host, int16_t master_port);

int gnet_create_master(const char* name, const char* host, int16_t port)
{

}

int gnet_send(gmsg* msg);

// return = 0, poll & message received
// return > 0, poll & nothing happened
int gnet_poll(gmsg* msg);

int gnet_release(const char* name);

// int luaopen_gnet(lua_State* L);
*/

