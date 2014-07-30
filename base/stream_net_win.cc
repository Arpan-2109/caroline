// Copyright (c) 2014 The Caroline authors. All rights reserved.
// Use of this source file is governed by a MIT license that can be found in the
// LICENSE file.
// Author: Konstantin Bormotov <bormotovk@gmail.com

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <memory>

#include "base/stream_net_win.h"
#include "base/logging.h"
#include "base/stream.h"


namespace {
  const enum {
    TCP = 1,
    UDP = 2
  };
}
namespace base {

  StreamNetWin::StreamNetWin(const std::string& ip_address,
    uint16_t port,
    Stream::Mode mode)
    : ip_addr_(ip_address),
    port_(port),
    mode_(mode),
    accepted_(false)
  {}
  StreamNetWin::~StreamNetWin() {
  }

  void StreamNetWin::InitWSA() {
    //if (!wsadata_)
    //  wsadata_ = new WSADATA();
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsadata_);   // Fill in WSA info
    if (iResult != NO_ERROR) {
      LOG(ERR) << WSAGetLastError() << "\n iResult: "<< iResult;
    }
  }

  void StreamNetWin::SetMode(SOCKET sock, Stream::Mode mode) {
    switch (mode) {
    case Stream::kRead: {
      shutdown(sock, SD_SEND);
      break;
    }
    case Stream::kWrite: {
      shutdown(sock, SD_RECEIVE);
      break;
    }
    case Stream::kAppend: {
      shutdown(sock, SD_RECEIVE);
      break;
    }
    case  Stream::kReadWrite: {
      shutdown(sock, SD_BOTH);
      break;
    }
    case Stream::kReadAppend: {
      shutdown(sock, SD_BOTH);
      break;
    }
    default: {
      break;
    }
    }
  }
  
  /*bool StreamNetWin::TryAccept() { 
    if (accepted_)
      return true;
    sockaddr addr_info;
    int size = sizeof(sockaddr);
    sock_ = accept(sock_, &addr_info, &size);
  }*/
  
  size_t StreamNetWin::Read(char* buffer, size_t size) {
    /*if (!TryAccept()) {
      return 0;
    }*/
    typedef int socklen_t;
    socklen_t from_lenghth;
    SOCKADDR_IN from = addr_info_;
    int received_bytes;
    if (socket_type_ == UDP) {
      received_bytes = recvfrom(sock_, buffer, size,
        0, reinterpret_cast<sockaddr*>(&from),
        &from_lenghth);
      if (received_bytes <= 0)
        Close();
    }
    else if (socket_type_ == TCP) {
      received_bytes = recv(sock_, buffer, size, 0);
      if (received_bytes <= 0) {
        Close();
      }
    }
    else {
      LOG(ERR) << "Reading error";
    }
    return received_bytes;
  }
  size_t StreamNetWin::Write(const char* buffer, size_t size) {
    /*if (!TryAccept()) {
      return 0;
    }*/
    int sent_bytes;
    if (socket_type_ == UDP) {
      sent_bytes = sendto(sock_, (const char*)buffer, size,
        0, reinterpret_cast<sockaddr*>(&addr_info_), sizeof(sockaddr_in));
      if (sent_bytes != size) {
        LOG(ERR) << "failed to send packet: return value " << sent_bytes;
        return false;
      }
    }
    else if (socket_type_ == TCP) {
      sent_bytes = send(sock_, buffer, size, 0);
      if (sent_bytes != size) {
        LOG(ERR) << "failed to send packet: return value " << sent_bytes;
        LOG(ERR) << "\n Error: " << WSAGetLastError();
        return false;
      }
      return true;
    }
    else {
      LOG(ERR) << "Writing error";
    }
    return true;
  }
  bool StreamNetWin::Seek(int offset, Stream::SeekType type) {
    return true;
  }
  size_t StreamNetWin::GetSize() {
    return 1;
  }
  void StreamNetWin::Close() {
    if (sock_)
      closesocket(sock_);
    WSACleanup();;
  }
  void StreamNetWin::SetSockAddr(int family,
                                 const std::string& ip_address,
                                 uint16_t port_no) {
    auto host_ip = ConvertAddr(ip_addr_);
    addr_info_.sin_family = AF_INET;
    addr_info_.sin_port = htons(port_no);
    addr_info_.sin_addr.s_addr = inet_addr(host_ip);
  }

  char* StreamNetWin::ConvertAddr(const std::string& ip_address) {
    hostent* hostaddr;
    char *host_ip;
    hostaddr = gethostbyname(ip_address.c_str());
    host_ip = inet_ntoa(*(struct in_addr *)*hostaddr->h_addr_list);
    return host_ip;
  }

  bool StreamNetWin::OpenTCPSocket() {
    StreamNetWin::InitWSA();
    socket_type_ = TCP;
    sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SetSockAddr(AF_INET, ip_addr_, port_);
    if (sock_ == INVALID_SOCKET) {
      LOG(ERR) << "Invalid Socket" << WSAGetLastError();
      Close();
      return false;
    }
    SetMode(sock_, mode_);
    AsyncThisShit();
    int iResult = connect(sock_,
      reinterpret_cast<SOCKADDR *>(&addr_info_),
      sizeof(addr_info_));
    if (iResult == SOCKET_ERROR) {
      LOG(ERR) << "SOCKET_ERROR" << WSAGetLastError();
      Close();
      return false;
    }
    else {
      return true;
    }
  }
  bool StreamNetWin::OpenUDPSocket() {
    StreamNetWin::InitWSA();
    StreamNetWin::SetSockAddr(AF_INET, ip_addr_, port_);
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_TCP);
    SetMode(sock_, mode_);
    if (sock_ == INVALID_SOCKET) {
      LOG(ERR) << "Invalid Socket" << WSAGetLastError();
      Close();
      return false;
    }
    else {
      return true;
    }
  }
  bool StreamNetWin::BindUDPSocket() {
    socket_type_ = UDP;
    StreamNetWin::InitWSA();
    DWORD non_blocking = 1;
    StreamNetWin::SetSockAddr(AF_INET, ip_addr_, port_);
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Create socket
    if (sock_ == INVALID_SOCKET) {
      LOG(ERR) << WSAGetLastError();
      return false;
    }
    SetMode(sock_, mode_);
    if (bind(sock_, (LPSOCKADDR)&addr_info_, sizeof(addr_info_)) == SOCKET_ERROR) {
      LOG(ERR) << WSAGetLastError();
      return false;
    }
    if (ioctlsocket(sock_, FIONBIO, &non_blocking) != 0) {
      LOG(ERR) << "Failed to set non-blocking socket \n";
      return false;
    }
    if (listen(sock_, SOMAXCONN) == SOCKET_ERROR) {
      LOG(ERR) << "listen function failed with error: %d\n" << WSAGetLastError();
      return false;
    }
    return true;
  }
  bool StreamNetWin::BindTCPSocket() {
    socket_type_ = TCP;
    DWORD non_blocking = 1;
    StreamNetWin::InitWSA();
    StreamNetWin::SetSockAddr(AF_INET, ip_addr_, port_);
    sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  // Create socket
    if (sock_ == INVALID_SOCKET)  {
      LOG(ERR) << WSAGetLastError();
      return false;
    }
    SetMode(sock_, mode_);
    if (bind(sock_, (LPSOCKADDR)&addr_info_, sizeof(addr_info_)) == SOCKET_ERROR) {
      LOG(ERR) << WSAGetLastError();
      return false;
    }
    /*if (ioctlsocket(sock_, FIONBIO, &non_blocking) != 0) {
      LOG(ERR) << "Failed to set non-blocking socket \n";
      return false;
    }*/
    AsyncThisShit();
    if (listen(sock_, SOMAXCONN) == SOCKET_ERROR) {
      LOG(ERR) << "listen function failed with error: %d\n" << WSAGetLastError();
      return false;
    }
    //SOCKET accept_socket_ = accept(sock_, NULL, NULL);
    //if (AcceptSocket == INVALID_SOCKET) {
    //  LOG(ERR) << "invalid accept socket: " << WSAGetLastError();
    //  return false;
    //}
    return true;
  }

  std::unique_ptr<Stream> Stream::OpenTCPSocket(
    const std::string& host,
    uint16_t port,
    Mode mode) {
    std::unique_ptr<StreamNetWin>
      connection(new StreamNetWin(host, port, mode));
    if (!connection->OpenTCPSocket())
      return nullptr;
    else
      return std::unique_ptr<Stream>(new Stream(std::move(connection)));
  }

  std::unique_ptr<Stream> Stream::OpenUDPSocket(const std::string& ip_address,
    uint16_t port,
    Mode mode) {
    std::unique_ptr<StreamNetWin>
      connection(new StreamNetWin(ip_address, port, mode));
    if (!connection->OpenUDPSocket())
      return nullptr;
    else
      return std::unique_ptr<Stream>(new Stream(std::move(connection)));
  }

  std::unique_ptr<Stream> Stream::BindTCPSocket(const std::string& ip_address,
    uint16_t port_no,
    Mode mode) {
    std::unique_ptr<StreamNetWin>
      connection(new StreamNetWin(ip_address, port_no, mode));
    if (!connection->BindTCPSocket())
      return nullptr;
    else
      return std::unique_ptr<Stream>(new Stream(std::move(connection)));
  }

  std::unique_ptr<Stream> Stream::BindUDPSocket(const std::string& ip_address,
    uint16_t port_no,
    Mode mode) {
    std::unique_ptr<StreamNetWin>
      connection(new StreamNetWin(ip_address, port_no, mode));
    if (!connection->BindUDPSocket())
      return nullptr;
    else
      return std::unique_ptr<Stream>(new Stream(std::move(connection)));
  }

  size_t Stream::Impl::Read(char *buffer, size_t size) {
    size_t recieved_bytes = Read(buffer, size);
    if (recieved_bytes < 0) {
      Close();
      return 0;
    }
    return recieved_bytes;
  }

  size_t Stream::Impl::Write(const char *buffer, size_t size) {
    size_t send_bytes = Write(buffer, size);
    if (send_bytes < 0) {
      Close();
      return 0;
    }
    return send_bytes;
  }

  bool Stream::Impl::Seek(int offset, SeekType type) {
    return Seek(offset, type);
  }
  size_t Stream::Impl::GetSize() {
    return GetSize();
  }


  void StreamNetWin::AsyncThisShit(){
    SOCKET sock = sock_;
    const int WM_SOCKET = 104;
    HWND console_wnd = GetConsoleWindow();
    int i_result = WSAAsyncSelect(sock, console_wnd, WM_SOCKET, (FD_CLOSE | FD_READ));
    if (i_result)
    {
      LOG(ERR) << "WSAAsyncSelect failed " << "Critical Error: " << WSAGetLastError();
    }
    else
      sock_ = sock;
  }
}  // namespace base