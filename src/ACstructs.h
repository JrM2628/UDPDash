/*
CREDIT : https://docs.google.com/document/d/1KfkZiIluXZ6mMhLWfDX1qAGbvhGRC3ZUzjVIt5FQpp4/pub
Data is sent from the ACServer following the form of one of these structures. 
*/
#define ACSERVERPORT 9996;
struct handshaker {
    int identifier;
    int version;
    int operationId;
};

//Default values on the website for the strings were 50, but changing them to 100 made everything function properly
//I think each character is 2 bytes instead of 1
struct handshakerResponse {
    char carName[100];
    char driverName[100];
    int identifier;
    int version;
    char trackName[100];
    char trackConfig[100];
};

struct RTCarInfo
{
    char identifier;
    int size;

    float speed_Kmh;
    float speed_Mph;
    float speed_Ms;

    bool isAbsEnabled;
    bool isAbsInAction;
    bool isTcInAction;
    bool isTcEnabled;
    bool isInPit;
    bool isEngineLimiterOn;


    float accG_vertical;
    float accG_horizontal;
    float accG_frontal;

    int lapTime;
    int lastLap;
    int bestLap;
    int lapCount;

    float gas;
    float brake;
    float clutch;
    float engineRPM;
    float steer;
    int gear;
    float cgHeight;

    float wheelAngularSpeed[4];
    float slipAngle[4];
    float slipAngle_ContactPatch[4];
    float slipRatio[4];
    float tyreSlip[4];
    float ndSlip[4];
    float load[4];
    float Dy[4];
    float Mz[4];
    float tyreDirtyLevel[4];

    float camberRAD[4];
    float tyreRadius[4];
    float tyreLoadedRadius[4];


    float suspensionHeight[4];

    float carPositionNormalized;

    float carSlope;

    float carCoordinates[3];
};

struct RTLap
{
    int carIdentifierNumber;
    int lap;
    char driverName[50];
    char carName[50];
    int time;
};