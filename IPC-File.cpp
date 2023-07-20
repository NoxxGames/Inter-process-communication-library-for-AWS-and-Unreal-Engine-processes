#include "IPCFile.h"

using namespace IPCFile;

/**
 * Debug Tests
 * TODO make a proper test harness
 */
int main(int argc, char* argv[])
{
    const IAttributeString PlayerAuth = IAttributeString(
        EAttributeName::PLAYER_AUTH, "TestPlayerAuthID238476981723");
    std::vector<EAttributeName> AttributesToGet;
    AttributesToGet.push_back(EAttributeName::IS_ONLINE);
    const FGetRequest GetRequest = FGetRequest(
        PlayerAuth,
        IPCFileManager::GenerateUniqueRequestID(),
        AttributesToGet);

    IPCFileManager::UE_Initialize();

    IPCFileManager::UE_AddGetRequestToBuffer(GetRequest);
    IPCFileManager::UE_WriteGetRequestBufferToFile(
        "C:\\Users\\josh_\\Documents\\IPCtest");

    FPlayerAttributeList AttList;
    AttList.SetPlayerAuthID(PlayerAuth);

    const FSetRequest SetRequest = FSetRequest(
        PlayerAuth,
        IPCFileManager::GenerateUniqueRequestID(),
        AttList);

    IPCFileManager::UE_AddSetRequestToBuffer(SetRequest);
    IPCFileManager::UE_AddSetRequestToBuffer(SetRequest);
    IPCFileManager::UE_AddSetRequestToBuffer(SetRequest);
    IPCFileManager::UE_AddSetRequestToBuffer(SetRequest);
    IPCFileManager::UE_AddSetRequestToBuffer(SetRequest);
    IPCFileManager::UE_AddSetRequestToBuffer(SetRequest);
    IPCFileManager::UE_AddSetRequestToBuffer(SetRequest);
    IPCFileManager::UE_AddSetRequestToBuffer(SetRequest);
    IPCFileManager::UE_AddSetRequestToBuffer(SetRequest);
    
    IPCFileManager::UE_WriteSetRequestBufferToFile(
        "C:\\Users\\josh_\\Documents\\IPCtest");
    
    system("pause");
    return 0;
}
