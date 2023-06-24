const char apiServer[] = "m.vitamin.spravazeleznic.cz";
const char msgServer[] = "provoz.spravazeleznic.cz";
#ifndef PROJECT_VERSION
  #define PROJECT_VERSION "0.0"
#endif
const char* root_ca = \
//We got this cert by exporting m.vitamin.spravazeleznic.cz's root cert in Jun23: DigiCert Global Root V2
 "-----BEGIN CERTIFICATE-----\n" \
 "MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n" \
 "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
 "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
 "MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
 "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
 "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n" \
 "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n" \
 "2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n" \
 "1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n" \
 "q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n" \
 "tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n" \
 "vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n" \
 "BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n" \
 "5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n" \
 "1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n" \
 "NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n" \
 "Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n" \
 "8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n" \
 "pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n" \
 "MrY=\n" \
 "-----END CERTIFICATE-----\n";

#define RQ_0HEAD_LENGTHIS \
  "POST /mvitamin/info/getInformationPanel HTTP/1.0\n" \
  "Content-Type: application/json; charset=UTF-8\n" \
  "Host: m.vitamin.spravazeleznic.cz\n" \
  "Connection: Close\n" \
  "Accept: */*\n" \
  "User-Agent: esp32/" "0.0" "\n" \
  "Content-Length: "

#define RQ_BASELENGTH 41
#define RQ_1STATIONIS "{\"stations\":[{\"sr70\":\""
#define RQ_2MODEIS "\",\"departure\":\""
#define DEP "DEPARTURE"
#define ARR "ARRIVAL"
#define RQ_3END "\"}]}"

#define RQSEARCH_1QUERY "{\"search\":\""
#define RQSEARCH_2END "\"}"

#define RQMSG \
  "POST /tabule/Pages/OG/JSMethod.aspx/GetMessage HTTP/1.0\n" \
  "Content-Type: application/json; charset=UTF-8\n" \
  "Host: provoz.spravazeleznic.cz\n" \
  "Connection: Keep-Alive\n" \
  "User-Agent: esp32/" "0.0" "\n" \
  "Content-Length: 0\n\n"

//curl --verbose "https://provoz.spravazeleznic.cz/tabule/Pages/OG/JSMethod.aspx/GetMessage" -X POST -H "User-Agent: esp/_._" -H "Content-Type: application/json; charset=utf-8" -H "Content-Length: 0"