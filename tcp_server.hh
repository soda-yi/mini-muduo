class TcpServer
{
public:
    TcpServer();
    ~TcpServer();
    void Start();

private:
    int CreateAndListen();
};