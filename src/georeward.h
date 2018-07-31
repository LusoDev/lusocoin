// Copyright (c) 2014-2017 The Dash Core developers
// Copyright (c) 2018 The Luso Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/*
 18-06-2018
 - Max Reward of 20% for PoW and 15% for MN
 - 10 Countries list: Portugal, Brazil, Angola, Mozambique, Guinea-Bissau, East Timor, Equatorial Guinea, Macau, Cape Verde, São Tomé and Príncipe
 - new geoip command
 - new checkluso command
 - Download IPv.gz data from official resources
 - GeoReward on MNs

*/

#include "tinyformat.h"
#include "timedata.h"
#include "util.h"
#include "utiltime.h"
#include <univalue.h>
#include "compat.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/asio.hpp>
#include <zlib.h>

using std::string;
using std::cout;
using std::endl;
using std::stringstream;

// Portuguese language speaking countries and regions
static const char * Nations[] = { "PT", "BR", "AO", "MZ", "GW", "TL", "CV", "GQ", "MO", "ST" };
static const char * IPvDB[] = { "lusoco.in", "explorer.lusoco.in", "bay.lusoco.in" }; // Temporary workaround

class CGEOReward {

private:

	char* appendCharToCharArray(char* array, char a) {
	    size_t len = strlen(array);

	    char* ret = new char[len+2];

	    strcpy(ret, array);
	    ret[len] = a;
	    ret[len+1] = '\0';

	    return ret;
	};
	std::string httpreq(const std::string& server, const std::string& file)
	{
		try
		{
			boost::asio::ip::tcp::iostream s(server, "http");
			s.expires_from_now(boost::posix_time::seconds(60));

			if (!s){ return "0"; }

			// ask for the file
			s << "GET " << file << " HTTP/1.0\r\n";
			s << "Host: " << server << "\r\n";
			s << "User-Agent: luso-web-cli\r\n";
			s << "Accept: */*\r\n";
			s << "Connection: close\r\n\r\n";

			// Check that response is OK.
			std::string http_version;
			s >> http_version;
			unsigned int status_code;
			s >> status_code;
			std::string status_message;
			std::getline(s, status_message);
			if (!s && http_version.substr(0, 5) != "HTTP/"){ return "0"; }
			if (status_code != 200){ return "0"; }

			// Process the response headers, which are terminated by a blank line.
			std::string header;
			while (std::getline(s, header) && header != "\r"){};
			if (strcmp(header.c_str(),"Content-Type: text/csv") != -1) return "0";
			// Write the remaining data to output.
			std::stringstream ss;
			ss << s.rdbuf();
			return ss.str();
		}
		catch(std::exception& e)
		{
			return e.what();
		}
	};

	bool is_ipv4_address(const string& str)
	{
			{
	        CNetAddr addr;
	        if (addr.SetSpecial(std::string(str))) {
	            return true;
	        }
	    }
#ifdef HAVE_GETADDRINFO_A
#ifdef HAVE_INET_PTON
			struct sockaddr_in sa;
			return inet_pton(AF_INET, str.c_str(), &(sa.sin_addr))!=0;
#else
			return inet_addr(str.c_str()) != INADDR_NONE;
#endif
#endif
	};

	bool is_ipv6_address(const string& str)
	{
			{
					CNetAddr addr;
					if (addr.SetSpecial(std::string(str))) {
							return true;
					}
			}
#ifdef HAVE_GETADDRINFO_A
#ifdef HAVE_INET_PTON
			struct sockaddr_in6 sa;
			return inet_pton(AF_INET6, str.c_str(), &(sa.sin6_addr))!=0;
#else
			return inet_addr(str.c_str()) != INADDR_NONE;
#endif
#endif
	};

	bool is_valid_domain_name(const string& str)
	{
					static const string tlds="|com|net|org|edu|gov|int|mil|ac|ad|ae|af|ag|ai|al|am|an|ao|aq|ar|arpa|at|au|ax|az|ba|bb|bd|be|bf|bg|bh|bi|bj|bm|bn|bo|br|bs|bt|bv|bw|by|bz|ca|cc|cd|cf|cg|ch|ci|ck|cl|cm|cn|co|cr|cu|cv|cx|cy|cz|de|dj|dk|dm|do|dz|ec|ee|eg|er|es|et|eu|fi|fj|fk|fm|fo|fr|ga|gb|gd|ge|gf|gg|gh|gi|gl|gm|gn|gp|gq|gr|gs|gt|gu|gw|gy|hk|hm|hn|hr|ht|hu|id|ie|il|im|in|io|iq|ir|is|it|je|jm|jo|jp|ke|kg|kh|ki|km|kn|kp|kr|kw|ky|kz|la|lb|lc|li|lk|lr|ls|lt|lu|lv|ly|ma|mc|md|me|mg|mh|mk|ml|mm|mn|mo|mp|mq|ms|mt|mu|mv|mw|mx|my|mz|na|nc|ne|nf|ng|ni|nl|no|np|nr|nu|nz|om|pa|pe|pf|pg|ph|pk|pl|pm|pn|pr|ps|pt|pw|py|qa|re|ro|rs|ru|rw|sa|sb|sc|sd|se|sg|sh|si|sj|sk|sl|sm|sn|so|sr|st|su|sv|sy|sz|tc|td|tf|tg|th|tj|tk|tl|tm|tn|to|tp|tr|tt|tv|tw|tz|ua|ug|uk|us|uy|uz|va|vc|ve|vg|vi|vn|vu|wf|ws|ye|yt|za|zm|zw|aw|as|mr|bl|bq|cw|eh|mf|ss|sx|um|aaa|aarp|abarth|abb|abbott|abbvie|abc|able|abogado|abudhabi|academy|accenture|accountant|accountants|acer|aco|active|actor|adac|ads|adult|aeg|aero|aetna|afamilycompany|afl|africa|africamagic|agakhan|agency|aig|aigo|airbus|airforce|airtel|akdn|alcon|alfaromeo|alibaba|alipay|allfinanz|allfinanzberater|allfinanzberatung|allstate|ally|alsace|alstom|amazon|americanexpress|americanfamily|amex|amfam|amica|amp|amsterdam|analytics|and|android|anquan|ansons|anthem|antivirus|aol|apartments|app|apple|aquarelle|aquitaine|arab|aramco|archi|architect|are|army|art|arte|asda|asia|associates|astrium|athleta|attorney|auction|audi|audible|audio|auspost|author|auto|autoinsurance|autos|avery|avianca|axa|axis|azure|baby|baidu|banamex|bananarepublic|band|bank|banque|bar|barcelona|barclaycard|barclays|barefoot|bargains|baseball|basketball|bauhaus|bayern|bbb|bbc|bbt|bbva|bcg|bcn|beats|beauty|beer|beknown|bentley|berlin|best|bestbuy|bet|bharti|bible|bid|bike|bing|bingo|bio|biz|black|blackfriday|blanco|blockbuster|blog|bloomberg|bloomingdales|blue|bms|bmw|bnl|bnpparibas|boats|boehringer|bofa|bom|bond|boo|book|booking|boots|bosch|bostik|boston|bot|boutique|box|bradesco|bridgestone|broadway|broker|brother|brussels|budapest|bugatti|buick|build|builders|business|buy|buzz|bway|bzh|cab|cadillac|cafe|cal|call|calvinklein|cam|camera|camp|canalplus|cancerresearch|canon|capetown|capital|capitalone|caravan|cards|care|career|careers|caremore|carinsurance|cars|cartier|casa|case|caseih|cash|cashbackbonus|casino|cat|catalonia|catering|catholic|cba|cbn|cbre|cbs|ceb|center|ceo|cern|cfa|cfd|chanel|changiairport|channel|charity|chartis|chase|chat|chatr|cheap|chesapeake|chevrolet|chevy|chintai|city|chk|chloe|christmas|chrome|chrysler|church|cialis|cimb|cipriani|circle|cisco|citadel|citi|citic|cityeats|claims|cleaning|click|clinic|clinique|clothing|club|clubmed|coach|codes|coffee|college|cologne|com|comcast|commbank|community|company|compare|computer|comsec|condos|connectors|construction|consulting|contact|contractors|cooking|cookingchannel|cool|coop|corsica|country|coupon|coupons|courses|credit|creditcard|creditunion|cricket|crown|crs|cruise|cruises|csc|cuisinella|cymru|cyou|dabur|dad|dance|data|date|dating|datsun|day|dclk|dds|deal|dealer|deals|degree|delivery|dell|delmonte|deloitte|delta|democrat|dental|dentist|desi|design|deutschepost|dhl|diamonds|diet|digikey|digital|direct|directory|discount|discover|dish|diy|dnb|dnp|docomo|docs|doctor|dodge|dog|doha|domains|doosan|dot|dotafrica|download|drive|dstv|dtv|dubai|duck|dunlop|duns|dupont|durban|dvag|dvr|dwg|earth|eat|eco|ecom|edeka|education|edu|email|emerck|emerson|energy|engineer|engineering|enterprises|epost|epson|equipment|ericsson|erni|esq|est|estate|esurance|etisalat|eurovision|eus|events|everbank|exchange|expert|exposed|express|extraspace|fage|fail|fairwinds|faith|family|fan|fans|farm|farmers|fashion|fast|fedex|feedback|ferrari|ferrero|fiat|fidelity|fido|film|final|finance|financial|financialaid|finish|fire|firestone|firmdale|fish|fishing|fitness|flickr|flights|flir|florist|flowers|fls|flsmidth|fly|foo|food|foodnetwork|football|ford|forex|forsale|forum|foundation|fox|free|fresenius|frl|frogans|frontdoor|frontier|ftr|fujitsu|fujixerox|fun|fund|furniture|futbol|fyi|gai|gal|gallery|gallo|gallup|game|games|gap|garden|garnier|gay|gbiz|gcc|gdn|gea|gecompany|ged|gent|genting|george|ggee|gift|gifts|gives|giving|glade|glass|gle|glean|global|globalx|globo|gmail|gmbh|gmc|gmo|gmx|godaddy|gold|goldpoint|golf|goo|goodhands|goodyear|goog|google|gop|got|gotv|grainger|graphics|gratis|gree|green|gripe|grocery|group|guardian|guardianlife|guardianmedia|gucci|guge|guide|guitars|guru|hair|halal|hamburg|hangout|haus|hbo|hdfc|hdfcbank|health|healthcare|heart|heinz|help|helsinki|here|hermes|hgtv|hilton|hiphop|hisamitsu|hitachi|hiv|hkt|hockey|holdings|holiday|homedepot|homegoods|homes|homesense|honda|honeywell|horse|host|hosting|hoteis|hotel|hoteles|hotels|hotmail|house|how|htc|hughes|hyatt|hyundai|ibm|icbc|ice|icu|idn|ieee|ifm|iinet|ikano|imamat|imdb|immo|immobilien|indians|industries|infiniti|info|infosys|infy|ing|ink|institute|insurance|insure|intel|international|intuit|investments|ipiranga|ira|irish|iselect|islam|ismaili|ist|istanbul|itau|itv|iveco|iwc|jaguar|java|jcb|jcp|jeep|jetzt|jewelry|jio|jlc|jll|jmp|jnj|jobs|joburg|jot|joy|jpmorgan|jpmorganchase|jprs|juegos|juniper|justforu|kaufen|kddi|kerastase|kerryhotels|kerrylogisitics|kerryproperties|ketchup|kfh|kia|kid|kids|kiehls|kim|kinder|kindle|kitchen|kiwi|koeln|komatsu|konami|kone|kosher|kpmg|kpn|krd|kred|ksb|kuokgroup|kyknet|kyoto|lacaixa|ladbrokes|lamborghini|lamer|lancaster|lancia|lancome|land|landrover|lanxess|lat|latino|latrobe|lawyer|lds|lease|leclerc|lefrak|legal|lego|lexus|lgbt|liaison|lidl|life|lifeinsurance|lifestyle|lighting|lightning|like|lilly|limited|limo|lincoln|linde|link|lipsy|live|livestrong|living|lixil|llc|loan|loans|locker|locus|loft|lol|london|loreal|lotte|lotto|love|lpl|lplfinancial|ltd|ltda|lundbeck|lupin|luxe|luxury|macys|madrid|maif|maison|makeup|man|management|mango|map|market|marketing|markets|marriott|marshalls|maserati|mattel|maybelline|mba|mcd|mcdonalds|mckinsey|media|medical|meet|melbourne|meme|memorial|men|menu|meo|merck|merckmsd|metlife|miami|microsoft|mih|mii|mini|mint|mit|mitek|mitsubishi|mlb|mls|mnet|mobi|mobile|mobily|moda|moe|mom|monash|money|monster|montblanc|mopar|mormon|mortgage|moscow|moto|motorcycles|mov|movie|movistar|mozaic|mrmuscle|mrporter|mtn|mtpc|mtr|multichoice|museum|music|mutual|mutualfunds|mutuelle|mzansimagic|nab|nadex|nagoya|name|naspers|nationwide|natura|navy|nba|nec|net|netaporter|netbank|netflix|network|neustar|new|newholland|news|next|nextdirect|nexus|nfl|ngo|nhk|nico|nike|nikon|ninja|nissan|nissay|nokia|northlandinsurance|northwesternmutual|norton|now|nowruz|nowtv|nra|nrw|ntt|nyc|obi|observer|off|okinawa|olayan|olayangroup|oldnavy|ollo|olympus|omega|ong|onl|online|onyourside|ooo|open|oracle|orange|org|organic|orientexpress|origins|osaka|otsuka|ott|overheidnl|ovh|page|pamperedchef|panasonic|panerai|paris|pars|partners|parts|party|passagens|patagonia|patch|pay|payu|pccw|persiangulf|pets|pfizer|pharmacy|phd|philips|phone|photo|photography|photos|physio|piaget|pics|pictet|pictures|pid|pin|ping|pink|pioneer|piperlime|pitney|pizza|place|play|playstation|plumbing|plus|pnc|pohl|poker|politie|polo|porn|post|pramerica|praxi|press|prime|pro|prod|productions|prof|progressive|promo|properties|property|protection|pru|prudential|pub|pwc|qpon|qtel|quebec|quest|qvc|racing|radio|raid|ram|read|realestate|realtor|realty|recipes|red|redken|redstone|redumbrella|rehab|reise|reisen|reit|ren|rent|rentals|repair|report|republican|rest|restaurant|retirement|review|reviews|rexroth|rich|richardli|ricoh|rightathome|ril|rio|rip|rmit|rocher|rocks|rockwool|rodeo|rogers|roma|room|rsvp|rugby|ruhr|run|rwe|ryukyu|saarland|safe|safety|safeway|sakura|sale|salon|samsclub|samsung|sandvik|sandvikcoromant|sanofi|sap|sapo|sapphire|sarl|sas|save|saxo|sbi|sbs|sca|scb|schaeffler|schedule|schmidt|scholarhips|scholarships|schule|schwarz|schwarzgroup|science|scjohnson|scor|scot|search|seat|security|seek|select|sener|services|ses|seven|sew|sex|sexy|sfr|shangrila|sharp|shell|shia|shiksha|shirriam|shoes|shop|shopping|shopyourway|shouji|show|showtime|shriram|silk|sina|singles|ski|skin|skolkovo|sky|skydrive|skype|sling|smart|smile|sncf|soccer|social|softbank|software|sohu|solar|solutions|song|sony|soy|spa|space|spiegel|sport|sports|spot|spreadbetting|srt|stada|staples|star|starhub|statebank|statefarm|statoil|stc|stcgroup|stockholm|storage|store|stream|stroke|studio|study|style|sucks|supersport|supplies|supply|support|surf|surgery|suzuki|svr|swatch|swiftcover|swiss|symantec|systems|sydney|tab|taipei|talk|taobao|target|tata|tatamotors|tatar|tattoo|tax|taxi|tci|tdk|team|technology|tel|telecity|telefonica|temasek|tennis|terra|teva|thai|thd|theater|theatre|theguardian|thehartford|tiaa|tickets|tienda|tiffany|tips|tires|tirol|tjmaxx|tjx|tkmaxx|tmall|today|tokyo|tools|top|toray|toshiba|total|tour|tours|town|toyota|toys|trade|tradershotels|trading|training|transformers|translations|transunion|travel|travelchannel|travelers|travelersinsurance|travelguard|trust|trv|tube|tui|tunes|tushu|tvs|ubank|ubs|uconnect|ultrabook|ummah|unicom|unicorn|university|uno|uol|ups|vacations|vana|vanguard|vanish|vegas|ventures|verisign|versicherung|vet|viajes|video|vig|viking|villas|vin|vip|virgin|visa|vision|vista|vistaprint|viva|vivo|vlaanderen|vodka|volkswagen|volvo|vons|vote|voting|voto|voyage|vuelos|wales|walmart|walter|wang|wanggou|warman|watch|watches|weather|weatherchannel|web|webcam|weber|webjet|webs|website|wed|wedding|weibo|weir|whoswho|wien|wiki|williamhill|wilmar|windows|wine|winners|wme|wolterskluwer|world|woodside|work|works|wow|wtc|wtf|xbox|xerox|xfinity|xihuan|xin|xn--11b4c3d|xn--1ck2e1b|xn--1qqw23a|xn--30rr7y|xn--3bst00m|xn--3ds443g|xn--3e0b707e|xn--3oq18vl8pn36a|xn--3pxu8k|xn--42c2d9a|xn--45brj9c|xn--45q11c|xn--4gbrim|xn--4gq48lf9j|xn--54b7fta0cc|xn--55qw42g|xn--55qx5d|xn--55qx5d8y0buji4b870u|xn--5su34j936bgsg|xn--5tzm5g|xn--6frz82g|xn--6qq986b3x1|xn--6qq986b3xl|xn--6rtwn|xn--80adxhks|xn--80ao21a|xn--80aqecdr1a|xn--80asehdb|xn--80aswg|xn--8y0a063a|xn--90a3ac|xn--9et52u|xn--9krt00a|xn--b4w605ferd|xn--bck1b9a5dre4c|xn--c1avg|xn--c1yn36f|xn--c2br7g|xn--cck2b3b|xn--cckwcxetd|xn--cg4bki|xn--clchc0ea0b2g2a9gcd|xn--czr694b|xn--czrs0t|xn--czru2d|xn--d1acj3b|xn--dkwm73cwpn|xn--eckvdtc9d|xn--efvy88h|xn--estv75g|xn--fct429k|xn--fes124c|xn--fhbei|xn--fiq228c5hs|xn--fiq64b|xn--fiQ64b|xn--fiQ64B|xn--fiqs8s|xn--fiqz9s|xn--fjq720a|xn--flw351e|xn--fpcrj9c3d|xn--fzc2c9e2c|xn--fzys8d69uvgm|xn--g2xx48c|xn--gckr3f0f|xn--gecrj9c|xn--gk3at1e|xn--h2brj9c|xn--hdb9cza1b|xn--hxt035cmppuel|xn--hxt035czzpffl|xn--hxt814e|xn--i1b6b1a6a2e|xn--imr513n|xn--io0a7i|xn--j1aef|xn--j1amh|xn--j6w193g|xn--j6w470d71issc|xn--jlq480n2rg|xn--jlq61u9w7b|xn--jvr189m|xn--kcrx77d1x4a|xn--kcrx7bb75ajk3b|xn--kprw13d|xn--kpry57d|xn--kpu716f|xn--kput3i|xn--lgbbat1ad8j|xn--mgb9awbf|xn--mgba3a3ejt|xn--mgba3a4f16a|xn--mgba7c0bbn0a|xn--mgbaakc7dvf|xn--mgbaam7a8h|xn--mgbab2bd|xn--mgbai9azgqp6j|xn--mgbayh7gpa|xn--mgbb9fbpob|xn--mgbbh1a71e|xn--mgbc0a9azcg|xn--mgbca7dzdo|xn--mgberp4a5d4ar|xn--mgbi4ecexp|xn--mgbt3dhd|xn--mgbv6cfpo|xn--mgbx4cd0ab|xn--mk1bu44c|xn--mxtq1m|xn--ngbc5azd|xn--ngbe9e0a|xn--ngbrx|xn--node|xn--nqv7f|xn--nqv7fs00ema|xn--nyqy26a|xn--o3cw4h|xn--ogbpf8fl|xn--otu796d|xn--p1acf|xn--p1ai|xn--pbt977c|xn--pgb3ceoj|xn--pgbs0dh|xn--pssy2u|xn--q9jyb4c|xn--qcka1pmc|xn--rhqv96g|xn--rovu88b|xn--s9brj9c|xn--ses554g|xn--t60b56a|xn--tckwe|xn--tiq49xqyj|xn--tqq33ed31aqia|xn--unup4y|xn--vermgensberater-ctb|xn--vermgensberatung-pwb|xn--vhquv|xn--vuq861b|xn--w4r85el8fhu5dnra|xn--w4rs40l|xn--wgbh1c|xn--wgbl6a|xn--xhq521b|xn--xkc2al3hye2a|xn--xkc2dl3a5ee0h|xn--yfro4i67o|xn--ygbi2ammx|xn--zfr164b|xn—3ds443g|xn—fiq228c5hs|xperia|xxx|xyz|yachts|yahoo|yamaxun|yandex|yellowpages|yodobashi|yoga|yokohama|you|youtube|yun|zappos|zara|zero|zip|zippo|zone|zuerich|zulu|";
					const char* s=str.c_str();
					int segsize=0,i,ix=str.length();
					if (s[0]=='.' || s[ix-1]=='.' || ix>253)
							return false;
					for(i=0,segsize=0; i<ix; i++)
					{
							if (s[i]=='.')
							{
									if (segsize==0) //fail for abc..com
											return false;
									segsize=0;
							}
							else if ( ('0' <= s[i] && s[i] <='9')
									|| ('a' <= s[i] && s[i] <='z')
									|| ('A' <= s[i] && s[i] <='Z')
									|| (s[i]=='-' && segsize!=0 && i+1<ix && s[i+1]!='.')
									)
							{
									segsize++;
							}
							else
									return false; //invalid char...

							if (segsize>63)
									return false;
					}

					stringstream ss;
					ss << "|" << str.substr(ix-segsize) << "|" ;//get last domain segment

					if (tlds.find(ss.str())==string::npos)
							return false;

					return true;
				};

        char* getfield(char* line, int num)
        {
            char* tok;
            for (tok = strtok(line, ",");
                    tok && *tok;
                    tok = strtok(NULL, ",\n"))
            {
                if (!--num)
                    return tok;
            }
            return NULL;
        }
        void removeChar(char *str, char garbage) {

            char *src;
            char *dst;
            for (src = dst = str; *src != '\0'; src++) {
                *dst = *src;
                if (*dst != garbage) dst++;
            }
            *dst = '\0';
        }
				bool decompressgz(const char* fileName,const char* fileOut) {
					gzFile inFileZ = gzopen(fileName, "rb");
					if (inFileZ == NULL) {
						const boost::filesystem::path tr = fileOut;
						boost::filesystem::ifstream streamConfig(tr);
						if (streamConfig.good()) {
							return 1;
						} else {
					    printf("Error: Failed to gzopen %s\n", fileName);
					    return 0;
						}
					}
					FILE * pFile = fopen (fileOut, "wb");
					unsigned char unzipBuffer[8192];
					unsigned int unzippedBytes;
					std::vector<unsigned char> unzippedData;
					while (true) {
					    unzippedBytes = gzread(inFileZ, unzipBuffer, 8192);
					    if (unzippedBytes > 0) {
        					fwrite(unzipBuffer, 1, unzippedBytes, pFile);
					    } else {
					        break;
					    }
					}
					gzclose(inFileZ);
					fclose (pFile);
					return 1;
				}
        bool downloadDB(std::string fl) {
						boost::filesystem::path fl_gz = fl + ".gz";
						boost::filesystem::path fullfl = GetDataDir() / fl;
						boost::filesystem::path fullfl_gz = GetDataDir() / (fl + ".gz");
						//printf("Downloading %s to %s\r\n",fl_gz.string().c_str(),fullfl_gz.string().c_str());
            bool downloaded=0;
            std::string result = "0";
						for(int i=0; i < 3; i++) {
							if (IPvDB[i] == NULL) break;
							result = httpreq(IPvDB[i],fl_gz.string().c_str());
							if (strcmp(result.c_str(),"0") != 0) break;

						}

            if (strcmp(result.c_str(),"0") != 0) {
							try {
								std::ofstream of(fullfl_gz.string().c_str(), std::ios::binary);
                of << result;
								MilliSleep(1000);
								if (decompressgz(fullfl_gz.string().c_str(),fullfl.string().c_str()) == 1) {
									remove(fullfl_gz.string().c_str());
									downloaded=1;
								}
							} catch(...){
								remove(fullfl.string().c_str());
								remove(fullfl_gz.string().c_str());
								return 0;
							}

            }
            return downloaded;

        }
public:
				void checkLoad()
				{
					const boost::filesystem::path s = GetDataDir() / "geoip.csv";
					checkLoad(s);
				}
        void checkLoad(const boost::filesystem::path csv)
        {
                boost::filesystem::ifstream streamConfig(csv);
                if (!streamConfig.good()) {
                        // Create empty luso.conf if it does not excist
												if (downloadDB("/geoip.csv") == 0) {
													printf("Failed to download GEOReward dependencies, are you connected to internet?\nTry to manually create geoip.csv.");
													throw;
												}
                }
								MilliSleep(1000);
								char* tst=getGEOIP("99.99.99.99");
								if (strcmp("US",tst) != 0) {
									printf("Failed to geolocate ip,\nTry to manually create geoip.csv.");
									throw;
								}
        };
        bool isLUSO()
        {
            char* Country=getGEOIP("127.0.0.1");
            for (int z = 0 ; z < 10; z ++) {
                if (strcmp(Nations[z],Country) == 0) {
                    return 1;
                }
            }
            return 0;
        };
        bool isLUSO(const char* addr)
        {
            char* Country=getGEOIP(addr);
            for (int z = 0 ; z < 10; z ++) {
                if (strcmp(Nations[z],Country) == 0) {
                    return 1;
                }
            }
            return 0;
        };
        bool isLUSO(const char* addr,char* Country)
        {
            for (int z = 0 ; z < 10; z ++) {
                if (strcmp(Nations[z],Country) == 0) {
                    return 1;
                }
            }
            return 0;
        };

        std::string checkLUSO(std::string& addr)
        {
                std::string str(checkLUSO((char *) addr.c_str()));
                return str;
        };

        char* checkLUSO(const char* addr)
        {
            char* Country=getGEOIP(addr);
            char* r=(char *) malloc(80);
            sprintf(r,"Failed, %s not in a Lusophonous network, geoip: \n\t%s\n",addr,Country);
            if (isLUSO(addr,Country) == 1) {
                sprintf(r,"Successful, %s in a Lusophonous network, geoip: \n\t%s\n",addr,Country);
            }
            return r;
        };
				char* getGEOcache(const char* addr) {
					int s1, s2, s3, s4;
          double c1, c2, c3, c4;
          double integer_ip;
          char* countryName=(char *) "Bad IP or not found.";
          sscanf(addr, "%d.%d.%d.%d", &s1, &s2, &s3, &s4);
          boost::filesystem::path fileIPv = GetDataDir() / ".geoip.cache";

          if (is_ipv4_address((std::string) addr)) {
                  c1 = (double)16777216 * s1;
                  c2 = (double)65536 * s2;
                  c3 = (double)256 * s3;
                  c4 = (double)s4;
                  integer_ip = c1 + c2 + c3 + c4;
          } else if (is_ipv6_address((std::string) addr)) {
                  // unavailable on MN, YET!

          } else
                return countryName;


					boost::filesystem::ifstream streamConfig(fileIPv);
					if (!streamConfig.good())
						return countryName;

					char* flopen = (char *) fileIPv.c_str();
					FILE* stream = fopen(flopen, "r");
          int found_it = 0;
          char* tmp;
          char line[1024];
          while (fgets(line, 1024, stream))
          {
                  tmp = strdup(line);
                  char* tmp2 = strdup(line);
                  char* tmp3 = strdup(line);

                  char* thirdField = getfield(tmp,3);
                  char* fourthField = getfield(tmp2,4);
                  removeChar(thirdField, '"');
                  removeChar(fourthField, '"');

                  double lower = atof(thirdField);
                  double upper = atof(fourthField);

                  double m = integer_ip;

                  if (m > lower && m < upper) {
                  found_it = 1;
                  countryName = getfield(tmp3,5);
                  removeChar(countryName, '"');
                  free(tmp);
									//fclose(stream);
									break;
                  }

                  if (m == lower || m == upper) {
                  found_it = 1;
                  countryName = getfield(tmp3,5);
                  removeChar(countryName, '"');
                  free(tmp);
									//fclose(stream);
									break;
                  }

          }

					return countryName;
				};
				void setGEOcache(const char* linetry) {
					const boost::filesystem::path cache = GetDataDir() / ".geoip.cache";
					boost::filesystem::ifstream streamConfig(cache);
					bool foundGEO=0;
					if (!streamConfig.good()) {
						unsigned int curLine = 0;
						string line;
						streamConfig.open(cache.string().c_str());
						if(streamConfig.is_open()) {
							while(getline(streamConfig, line)) {
							    curLine++;
							    if (strcmp(line.c_str(),linetry) == 0) {
							        foundGEO=1;
											break;
							    }
							}
							//streamConfig.close();
						}

					}

					if (foundGEO == 0) {
						std::ofstream of(cache.string().c_str(), std::fstream::out | std::ios::binary);
						of << linetry;
						/*
						of.close();
						if (curLine < 201) {

						}
						*/
					}
				};
        std::string getGEOIP(std::string& addr)
        {
                std::string str(getGEOIP((char *) addr.c_str()));
                return str;
        };
				char* fixIP(const char* addrg) {
					int ip1, ip2, ip3, ip4, port;
					char* readdr = const_cast<char *>(addrg);
					if (strpbrk(readdr, ":") != 0) {
						sscanf(addrg, "%d.%d.%d.%d:%d", &ip1, &ip2, &ip3, &ip4, &port);
						sprintf(readdr,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
					}
					return readdr;

				}
        char* getGEOIP(const char* addrg)
        {
					char* addr = fixIP(addrg);
					char* tryCache=getGEOcache(addr);
					if (strcmp(tryCache,"Bad IP or not found.") != 0) {
						return tryCache;
					}
          int s1, s2, s3, s4;
          double c1, c2, c3, c4;
          double integer_ip;
          char* countryName=(char *) "Bad IP or not found.";
          sscanf(addr, "%d.%d.%d.%d", &s1, &s2, &s3, &s4);
          boost::filesystem::path fileIPv = GetDataDir() / "geoip.csv";

          if (is_ipv4_address((std::string) addr)) {
                  c1 = (double)16777216 * s1;
                  c2 = (double)65536 * s2;
                  c3 = (double)256 * s3;
                  c4 = (double)s4;
                  integer_ip = c1 + c2 + c3 + c4;
          } else if (is_ipv6_address((std::string) addr)) {
                  // unavailable on MN, YET!

          } else
                return countryName;

					boost::filesystem::ifstream streamConfig(fileIPv);
						if (!streamConfig.good())
							return countryName;

					char* flopen = (char *) fileIPv.c_str();
          FILE* stream = fopen(flopen, "r");
          int found_it = 0;
          char* tmp;
          char line[1024];
          while (fgets(line, 1024, stream))
          {
                  tmp = strdup(line);
                  char* tmp2 = strdup(line);
                  char* tmp3 = strdup(line);

                  char* thirdField = getfield(tmp,3);
                  char* fourthField = getfield(tmp2,4);
                  removeChar(thirdField, '"');
                  removeChar(fourthField, '"');

                  double lower = atof(thirdField);
                  double upper = atof(fourthField);

                  double m = integer_ip;

                  if (m > lower && m < upper) {
	                  found_it = 1;
	                  countryName = getfield(tmp3,5);
	                  removeChar(countryName, '"');
	                  free(tmp);
										//fclose(stream);
										break;
                  }

                  if (m == lower || m == upper) {
	                  found_it = 1;
	                  countryName = getfield(tmp3,5);
	                  removeChar(countryName, '"');
	                  free(tmp);
										//fclose(stream);
										break;
                  }

          }

          if (found_it != 0) {
						const char* tmpln=line;
						setGEOcache(tmpln);
					}
          return countryName;
        };

};
