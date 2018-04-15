#include "packet.h"
#include <QRegExp>
#include <QDebug>

HTTP::HTTP(){}
HTTP::~HTTP(){}
HTTP::HTTP(QString text)
{
    QRegExp httpGetMethodReg("GET .+\r\n");
    httpGetMethodReg.setMinimal(true);
    QRegExp httpHostReg("Host: .+\r\n");
    httpHostReg.setMinimal(true);
    QRegExp httpConnectionReg("Connection: .+\r\n");
    httpConnectionReg.setMinimal(true);
    QRegExp httpCacheControlReg("Cache-Control: .+\r\n");
    httpCacheControlReg.setMinimal(true);
    QRegExp httpUserAgentReg("User-Agent: .+\r\n");
    httpUserAgentReg.setMinimal(true);
    QRegExp httpAcceptReg("Accept: .+\r\n");
    httpAcceptReg.setMinimal(true);
    QRegExp httpResponseReg("HTTP/1.1 .+\r\n");
    httpResponseReg.setMinimal(true);

    if (httpGetMethodReg.indexIn(text) > -1)      httpMethod = httpGetMethodReg.cap(0);
    if (httpHostReg.indexIn(text) > -1)           httpHost = httpHostReg.cap(0);
    if (httpConnectionReg.indexIn(text) > -1)     httpConnection = httpConnectionReg.cap(0);
    if (httpCacheControlReg.indexIn(text) > -1)   httpCacheControl = httpCacheControlReg.cap(0);
    if (httpUserAgentReg.indexIn(text) > -1)      httpUserAgent = httpUserAgentReg.cap(0);
    if (httpAcceptReg.indexIn(text) > -1)         httpAccept = httpAcceptReg.cap(0);
    if (httpResponseReg.indexIn(text) > -1)       httpResponse = httpResponseReg.cap(0);
}


QString analyzeHttpPacket(struct Packet *Pindex)
{
    char* ip_pkt_data = (char*)Pindex->IPv4_header;
    int ip_len = ntohs(Pindex->IPv4_header->tlen);
    bool find_http = false;
    std::string http_txt = "";
    for(int i=0;i<ip_len;++i){

        //check the http request
        if(!find_http
                && ((i+3<ip_len && strncmp(ip_pkt_data+i,"GET ",strlen("GET ")) ==0 )
                || (i+4<ip_len && strncmp(ip_pkt_data+i,"HEAD ",strlen("HEAD ")) == 0 )
                || (i+4<ip_len && strncmp(ip_pkt_data+i,"POST ",strlen("POST ")) == 0 )
                || (i+3<ip_len && strncmp(ip_pkt_data+i,"PUT ",strlen("PUT ")) == 0 )
                || (i+6<ip_len && strncmp(ip_pkt_data+i,"OPTION ",strlen("OPTION ")) == 0 ))
                )
        {
            find_http = true;
        }

        //check the http response
        if(!find_http && i+8<ip_len && strncmp(ip_pkt_data+i,"HTTP/1.1 ",strlen("HTTP/1.1 "))==0)
        {
            find_http = true;
        }

        //collect the http text
        if(find_http && (isalnum(ip_pkt_data[i]) || ispunct(ip_pkt_data[i]) || \
                         isspace(ip_pkt_data[i]) || isprint(ip_pkt_data[i])))
        {
            http_txt += ip_pkt_data[i];
        }
    }
    Pindex->Netpro = "HTTP";
    if (find_http)
        return QString(http_txt.c_str());
    else
        return QString("");
}

IP::~IP(){}

IP::IP(ip_header *ih)
{
    ip_hdr = ih;
    ver = QString::number((ntohs(ip_hdr->ver_ihl)&0xf000)>>12);
    // qDebug() << "Version1: "<< ver;
    hdr_len =QString::number((ntohs(ip_hdr->ver_ihl)&0x0f00)>>8);
    // qDebug() << "Header len: "<< hdr_len;
    tos = QString::number(ntohs(ip_hdr->tos));
//    tlen = QString::number(ip_hdr->tlen);
//    qDebug() << "Total Len1: "<< tlen;
    tlen = QString::number(ntohs(ip_hdr->tlen));
//    qDebug() << "Total Len2: "<< tlen;
    flags = QString::number((ntohs(ip_hdr->flags_fo)&0xe000)>>13);
    ttl = QString::number(ip_hdr->ttl);
//    qDebug()<< "TTL1: " << ttl;
//    ttl = QString::number(ntohs(ip_hdr->ttl));
//    qDebug() << "TTL2: " << ttl;
    proto = QString::number(ip_hdr->proto);
    src = QString("%1.%2.%3.%4")\
            .arg(ip_hdr->saddr.byte1)\
            .arg(ip_hdr->saddr.byte2)\
            .arg(ip_hdr->saddr.byte3)\
            .arg(ip_hdr->saddr.byte4);
    dst =  QString("%1.%2.%3.%4")\
            .arg(ip_hdr->daddr.byte1)\
            .arg(ip_hdr->daddr.byte2)\
            .arg(ip_hdr->daddr.byte3)\
            .arg(ip_hdr->daddr.byte4);
    crc = QString::number(ip_hdr->crc);
    ident = QString::number(ip_hdr->identification);
    qDebug() << "############# IP INFO #############";
    qDebug() << "Version: " << ver;
    qDebug() << "Header len: "<< hdr_len;
    qDebug() << "Type of service： " << tos;
    qDebug() << "total len: " << tlen;
    qDebug() << "flags : "<< flags;
    qDebug() <<"TTL: "<<ttl;
    qDebug() <<"Protocol: " << proto;
    qDebug() <<"Source: "<< src;
    qDebug() << "Destination: " <<dst;
    qDebug() << "CRC: " << crc;
    qDebug() << "Identical: " << ident;
}

TCP::~TCP()
{

}

TCP::TCP(tcp_header *th)
{
    tcp_hdr = th;
    src_port = QString::number(ntohs(tcp_hdr->sport));
    dst_port = QString::number(ntohs(tcp_hdr->dport));
    seq_num = QString::number(ntohs(tcp_hdr->seq));
    ack_num = QString::number(ntohs(tcp_hdr->ack));
    data_offset = QString::number((ntohs(tcp_hdr->tcp_res)&0xf000)>>12);
    flags = ntohs(tcp_hdr->tcp_res)&0x003f;
    URG = flags & 0x0020;
    ACK = flags & 0x0010;
    PSH = flags & 0x0008;
    RST = flags & 0x0004;
    SYN = flags & 0x0002;
    FIN = flags & 0x0001;
    window_size = QString::number(ntohs(tcp_hdr->windsize));
    crc = QString::number(ntohs(tcp_hdr->crc));
    urgp = QString(ntohs(tcp_hdr->urgp));

    qDebug() << "############# TCP INFO #############";
    qDebug() << "Src port : "<< src_port;
    qDebug() << "Dst port: " << dst_port;
    qDebug() << "seq number : "<< seq_num;
    qDebug() << "ack number : " << ack_num;
    qDebug() << "data offset: " << data_offset;
    qDebug() << "flags : " << flags;
    qDebug() << QString("URG:%1,ACK:%2,PSH:%3,RST:%4,SYN:%5,FIN:%6;").arg(URG).arg(ACK).arg(PSH).arg(RST).arg(SYN).arg(FIN);
    qDebug() << "Window size: " << window_size;
    qDebug() << "Checksum : "<< crc;
    qDebug() << "Urg pointer: "<< urgp;
}

UDP::~UDP()
{

}

UDP::UDP(udp_header *uh)
{
    udp_hdr = uh;
    src_port = QString::number(ntohs(udp_hdr->sport));
    dst_port = QString::number(ntohs(udp_hdr->dport));
    length = QString::number(ntohs(udp_hdr->len));
    crc = QString::number(ntohs(udp_hdr->crc));

    qDebug() << "############# UDP INFO #############";
    qDebug() << "Src port : "<< src_port;
    qDebug() << "Dst port: " << dst_port;
    qDebug() << "Length: "<< length;
    qDebug() << "Checksum: "<< crc;
}
