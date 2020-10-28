/*
 * testRapidJson.cpp
 *
 *  Created on: May 13, 2019
 *      Author: gyd
 */

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"

#include <sys/stat.h>           // mkdir stat
#include <iostream>
#include <memory>               // shared_ptr
using namespace std;
using namespace rapidjson;

int getFileSize(const char * filename)
{
    struct stat buf;
    long size = 0;
    if((stat(filename,&buf)!=-1))
    {
        size = buf.st_size;
    }
    else
    {
        printf("getFileSize failed: %d - %s\n" , errno, strerror(errno));;
        return -1;
    }
    return size;
}
void CloseFile(FILE *fp)
{
    if(fp == nullptr)
        return;
    fclose(fp);
    fp = nullptr;
    printf("file closed\n");
}
int testRapidJson() {
    char filename[256] = "config.json";
    long fileSize = getFileSize(filename);

    FILE *fp = fopen(filename, "r");
    shared_ptr<FILE> fileptr(fp, CloseFile);
    if(fp == NULL) {
        perror("Open failed.");
        return -1 ;
    }
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    if (fileSize < 4)  // readStream will get an error
    {
        cout << "fileSize < 4" << endl;
        return -1;
    }
    char *json = new char[fileSize + 64];
    shared_ptr<char> buffer(json, std::default_delete<char[]>());
    fseek(fp, 0, SEEK_SET);
//    fread(json, sizeof(char), fileSize + 1, fp);
    // windows下使用fread读取到的内容会解析失败，要用 FileReadStream
    FileReadStream readStream(fp, json, fileSize);
    Document doc;
    if (doc.ParseStream(readStream).HasParseError())
    {
        cout << "parse error" << endl;
        return 1;
    }
    // get string of an object
    Value::ConstMemberIterator iter;
    iter = doc.FindMember("Device");           // can be faster
    if (iter != doc.MemberEnd())
    {
        Value &device = doc["Device"];
        if(device.IsArray() == true )
        {
            printf("Is Array\n");
            for (SizeType i = 0; i < device.Size(); i++) // 使用 SizeType 而不是 size_t
                printf("Device[%d] : %s %s %s-%s %s %s\n", i,
                        device[i].FindMember("date")->value.GetString(),
                        device[i].FindMember("uvpa")->value.GetString(),
                        device[i].FindMember("cmd")->value.GetString(),
                        device[i].FindMember("subcmd")->value.GetString(),
                        device[i].FindMember("srcMac")->value.GetString(),
                        device[i].FindMember("dstMac")->value.GetString()
                        );
        }
    }

    // Modify it by DOM.
    if(doc.HasMember("VirtualMac") == true )
    {
        cout << doc["VirtualMac"].GetString()
                << " len: " << doc["VirtualMac"].GetStringLength()
                << " type: " << doc["VirtualMac"].GetType() << endl;

        Value& stat = doc["VirtualMac"];
        stat.SetString("00:11:22:33:44:55");
    }
    //
    iter = doc.FindMember("VirtualNum");        // can be faster
    if (iter != doc.MemberEnd())
        printf("%s\n", iter->value.GetString());
    else
        cout << "No VirtualNum" << endl;
    //
    iter = doc.FindMember("TUN_NAME");
    if (iter != doc.MemberEnd())
        printf("%s\n", iter->value.GetString());
    else
        cout << "No TUN_NAME" << endl;
    //
    iter = doc.FindMember("TUN_IP");
    if (iter != doc.MemberEnd())
        printf("%s\n", iter->value.GetString());
    else
        cout << "No TUN_IP" << endl;
    //
    iter = doc.FindMember("TUN_NETMASK");
    if (iter != doc.MemberEnd())
        printf("%s\n", iter->value.GetString());
    else
        cout << "No TUN_NETMASK" << endl;
    //
    iter = doc.FindMember("TUN_GATEWAY");
    if (iter != doc.MemberEnd())
        printf("%s\n", iter->value.GetString());
    else
        cout << "No TUN_GATEWAY" << endl;
    //
    iter = doc.FindMember("TUN_MTU");
    if (iter != doc.MemberEnd())
        printf("%s\n", iter->value.GetString());
    else
        cout << "No TUN_MTU" << endl;
    // add member
    Document::AllocatorType& allocator = doc.GetAllocator();

    Value object1(kObjectType);
    object1.AddMember("IP",   "10.0.0.2",    allocator);
    object1.AddMember("os",       "linux",   allocator);
    object1.AddMember("mac",      "00:22",    allocator);
    object1.AddMember("virmac",   "ff01",    allocator);
    object1.AddMember("virnum",   "0002",    allocator);

    Value object2(kObjectType);
    object2.AddMember("IP",   "10.0.0.3",    allocator);
    object2.AddMember("os",    "8f85",       allocator);
    object2.AddMember("mac",    "00:ff",     allocator);
    object2.AddMember("virmac",    "00:33",   allocator);
    object2.AddMember("virnum",    "ef21",   allocator);
    // add array
    Value& lines = doc["Device"];   // This time we uses non-const reference. doc["Device"] must be exist
    lines.SetArray();
    lines.PushBack(object1, allocator);
    lines.PushBack(object2, allocator);
    // Fluent API
    //lines.PushBack(object1, allocator).PushBack(object2, allocator);

    // Stringify the DOM
    StringBuffer buffer1;
//    Writer<StringBuffer> writer(buffer);    //write filtered the blanks
//    doc.Accept(writer);
    PrettyWriter<StringBuffer> pretty_writer(buffer1);
    doc.Accept(pretty_writer);
    // Output {"project":"rapidjson","stars":11}
    std::cout << buffer1.GetString() << std::endl;

    // get array
    iter = doc.FindMember("Device");           // can be faster
    if (iter != doc.MemberEnd())
    {
        Value& records = doc["Device"];
        if(records.IsArray() == true )
        {
            for (SizeType i = 0; i < records.Size(); i++) // 使用 SizeType 而不是 size_t
                printf("path[%d] : %s %s-%s %s %s\n", i, records[i].FindMember("IP")->value.GetString(),
                        records[i].FindMember("os")->value.GetString(),
                        records[i].FindMember("mac")->value.GetString(),
                        records[i].FindMember("virmac")->value.GetString(),
                        records[i].FindMember("virnum")->value.GetString() );
        }
    }
    return 0;
}


