const char serverName[] = "provoz.spravazeleznic.cz";

const char root_ca[] = \
//We got this cert by exporting provoz.spravazeleznic.cz's root cert in Apr23: DigiCert Global Root V2
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

const char postBaseURL[] = \
  "POST /tabule/Pages/StationTable.aspx?Key="
;

const char httpRequestHeaders[] = \
  " HTTP/1.1\n" \
  "Host: provoz.spravazeleznic.cz\n" \
  "Accept: *""/""*\n" \
  //divided * / * to prevent messing with comment
//"Accept-Encoding: gzip\n" \  	
  ///G-unZIP --not-- implemented in this version
  "User-Agent: esp32/0.0\n" \
  //not required; out of courtesy
  "Connection: keep-alive\n" \
  //we will see if this affects performance
  "content-type: application/x-www-form-urlencoded\n" \
  "Content-Length: ";
//Cookie: 	_shibsession_6465********************************************************************************************************7468=_cb7a************************c251; ASP.NET_SessionId=n244****************iq2w; sess=B15E************************76F5
const char httpRequestData[] = \
 //Approx. shortest possible HTTP request data
 "__EVENTTARGET" \
 //Tento parametr musi byt pritomen, nemusi mu byt prirazena hodnota
 "&__CALLBACKID=ctl00\04524OGMainContent\04524BCPTable" \
 //Tento parametr musi mit presnou hodnotu
 "&__CALLBACKPARAM=c0\0453A" \
 //Tento parametr musi mit presnou hodnotu
 "&__EVENTVALIDATION=Dobry\0452Bden\0452F\0452Bnechci\0452BDoSovat\0452BVasi\0452Bstranku\0452F\0452B" \
 "jen\0452Bsi\0452Beticky\0452Bvyrobit\0452Bodjezdovou\0452Btabuli\0452Bs\0452BESP32\0452B" \
 "pro\0452Bmodelovou\0452Bzeleznici\0452F\0452B" \
 "Zdrojak\0452Ba\0452Bkontakt\0452Bbude\0452Bna\0452BGitHubu\0452F\0452BNo\0452Bban\0452Bplz\0452BUwU\0452F\0453D";
 //Tento parametr musi byt dostatecne dlouha base64 hodnota, na obsahu nezalezi, nemusi byt ani unikatni
 /*Tak jsem ho vyuzil ke kratke zprave, pomoci base64-url lze dekodovat na
     Dobry-den_-nechci-DoSovat-Vasi-stranku_-jen-si-eticky-vyrobit-odjezdovou-tabuli-s-ESP32-pro-modelovou-zeleznici_-
     Zdrojak-a-kontakt-bude-na-GitHubu_-No-ban-plz-UwU_
 */

/*
POST request s daty vyse, jak ho delam v main.cpp na radku 32, muzete na PC zreplikovat pomoci 
    curl "https://provoz.spravazeleznic.cz/tabule/Pages/StationTable.aspx?Key=3223" -X POST --data-raw "__EVENTTARGET&__CALLBACKID=ctl00%24OGMainContent%24BCPTable&__CALLBACKPARAM=c0%3A&__EVENTVALIDATION=Dobry%2Bden%2F%2Bnechci%2BDoSovat%2BVasi%2Bstranku%2F%2Bjen%2Bsi%2Beticky%2Bvyrobit%2Bodjezdovou%2Btabuli%2Bs%2BESP32%2Bpro%2Bmodelovou%2Bzeleznici%2F%2BZdrojak%2Ba%2Bkontakt%2Bbude%2Bna%2BGitHubu%2F%2BNo%2Bban%2Bplz%2BUwU%2F%3D" -v --output -
Funguje i na strojich, ktere se na provoz.spravazeleznic.cz jeste nikdy nepripojily, mohu tedy vyloucit potrebu cookies
Vystup na mem stroji:
     Note: Unnecessary use of -X or --request, POST is already inferred.
     *   Trying 213.235.156.4:443...
     * Connected to provoz.spravazeleznic.cz (213.235.156.4) port 443 (#0)
	 * ALPN: offers h2,http/1.1
	 * TLSv1.3 (OUT), TLS handshake, Client hello (1):
	 *  CAfile: C:\bin\curl-ca-bundle.crt
	 *  CApath: none
	 * TLSv1.3 (IN), TLS handshake, Server hello (2):
	 * TLSv1.2 (IN), TLS handshake, Certificate (11):
	 * TLSv1.2 (IN), TLS handshake, Server key exchange (12):
	 * TLSv1.2 (IN), TLS handshake, Server finished (14):
	 * TLSv1.2 (OUT), TLS handshake, Client key exchange (16):
	 * TLSv1.2 (OUT), TLS change cipher, Change cipher spec (1):
	 * TLSv1.2 (OUT), TLS handshake, Finished (20):
	 * TLSv1.2 (IN), TLS handshake, Finished (20):
	 * SSL connection using TLSv1.2 / ECDHE-RSA-AES256-GCM-SHA384
	 * ALPN: server accepted h2
	 * Server certificate:
	 *  subject: CN=provoz.spravazeleznic.cz
	 *  start date: Jun 29 00:00:00 2022 GMT
	 *  expire date: Jul  7 23:59:59 2023 GMT
	 *  subjectAltName: host "provoz.spravazeleznic.cz" matched cert's "provoz.spravazeleznic.cz"
	 *  issuer: C=US; O=DigiCert Inc; OU=www.digicert.com; CN=GeoTrust TLS RSA CA G1
	 *  SSL certificate verify ok.
	 * using HTTP/2
	 * h2h3 [:method: POST]
	 * h2h3 [:path: /tabule/Pages/StationTable.aspx?Key=3223]
	 * h2h3 [:scheme: https]
	 * h2h3 [:authority: provoz.spravazeleznic.cz]
	 * h2h3 [user-agent: curl/8.0.1]
	 * h2h3 [accept: *\/*]
	 * h2h3 [content-length: 328]
	 * h2h3 [content-type: application/x-www-form-urlencoded]
	 * Using Stream ID: 1 (easy handle 0xaed41a35e0)
	 > POST /tabule/Pages/StationTable.aspx?Key=3223 HTTP/2
	 > Host: provoz.spravazeleznic.cz
	 > user-agent: curl/8.0.1
	 > accept: *\/*
	 > content-length: 328
	 > content-type: application/x-www-form-urlencoded
	 >
	 * We are completely uploaded and fine
	 < HTTP/2 200
	 < cache-control: no-cache, no-store
	 < pragma: no-cache
	 < content-type: text/plain; charset=utf-8
	 < expires: -1
	 < server: Microsoft-IIS/10.0
	 < set-cookie: ASP.NET_SessionId=0kfiyadu55moms3vrhijqe1v; path=/; HttpOnly; SameSite=Lax
	 < x-aspnet-version: 4.0.30319
	 < x-powered-by: ASP.NET
	 < date: Thu, 27 Apr 2023 09:43:09 GMT
	 < content-length: 80074
	 <
(nasleduje asi 80 kB HTML, ktere tvori tabuli odjezdu)
*/