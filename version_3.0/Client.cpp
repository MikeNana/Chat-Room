#include "Client.h"
#include "Channel.h"
#include "Helper.h"
#include "EventLoop.h"
#include <functional>
#include <memory>
#include <sys/unistd.h>
#include <iostream>
#include <sys/epoll.h>
#include <mutex>
#include <condition_variable>
#include <thread>

using std::shared_ptr;
using std::bind;
using std::cout;
using std::endl;
using std::unique_lock;

const int MAX_BUFF = 1000;

Client::Client(EventLoop* loop, int connfd)
    :
    connfd_(connfd),
    loop_(loop),
    channel_(new Channel(loop_, connfd_))
{
    channel_->set_readhandler(bind(&Client::handle_read, this));
    channel_->set_writehandler(bind(&Client::handle_write, this));
    channel_->set_connhandler(bind(&Client::handle_conn, this));
}

Client::~Client(){}

void Client::seperate_timer()
{
    if(timer_.lock())
    {
        shared_ptr<TimerNode> SP_Timer(timer_.lock());
        //SP_Timer->clean_req();
        timer_.reset();
    }
}

void Client::handle_read()
{
    cout << "from loop " << get_loop()->get_epollfd_() << endl;
    ssize_t read_len = 0;
    bool zero = false;
    EventLoop* loop_ = get_loop();
    read_len = read_str(connfd_, loop_->buffer_, zero);
    if(read_len < 0)
    {
        handle_error("read failed");
    }
//客户端主动close/ctrl+c/kill
    else if(read_len == 0)
    {
        cout << "client shutdown." << endl;
        system("pause");
    }
//具体格式后面修改
    cout << loop_->buffer_ << endl;
}

void Client::handle_write()
{
    ssize_t write_len = 0;
    if((write_len = write_str(connfd_, out_buffer_)) < 0)
    {
        handle_error("write failed");
    }
//后面改具体格式
    cout << out_buffer_ << endl;
}

void Client::handle_conn()
{
    seperate_timer();
    loop_->handle_curconn(channel_);
}
void Client::new_conn()
{
    channel_->set_events(EPOLLIN | EPOLLET);
    loop_->handle_newconn(channel_, 0);
}