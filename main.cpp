#include <iostream>
#include <raylib.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/ref.hpp>
#include <iomanip>
#include <chrono>
#include <ctime> 

#if defined(_WIN32)           
	#define NOGDI             // All GDI defines and routines
	#define NOUSER            // All USER defines and routines
#endif


#include <cpr/cpr.h>

#if defined(_WIN32)           // raylib uses these names as function parameters
	#undef near
	#undef far
#endif

int mouseX;
int mouseY;


Font fontDefault ; // font is global, so I don't need to pass reference to every object



class Button {
    public:
        
        int xloc;
        int yloc;
        int width;
        int height;
        float fontSize;
        std::string text;
        Color color;
        void Draw(){
            
            
            
            DrawRectangle(xloc,yloc,width,height,color);
            DrawRectangle(xloc+1,yloc+1,width-2,height-2,WHITE);
            DrawRectangle(xloc+2,yloc+2,width-4,height-4,color);
            Vector2 sz = MeasureTextEx(fontDefault,text.c_str(),fontSize,1);
            DrawTextEx(fontDefault,text.c_str(),Vector2({float(xloc+width/2-sz.x/2),float(yloc+height/2-sz.y/2)}),fontSize,1,WHITE);
        
        }
        
        bool IsClicked(){
            return ((mouseX>xloc && mouseX<(xloc+width))&&(mouseY>yloc && mouseY<(yloc+height)))&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        }
        
};

class InputBox{
    public:
        int xloc;
        int yloc;
        int width;
        int height;
        bool isSelected;
        std::string mode = "numbers";
        std::string text = "";
        void Draw()
        {
            if(isSelected)
            {
                
                DrawRectangle(xloc,yloc,width,height,RED);
                if(mode =="numbers")
                {
                    int key = GetCharPressed(); //Get char pressed (unicode character) on the queue
                
                    if((key>=48 && key<=58)||(key==46||key==44)||(key==45)) //48(0) to 57(9) or 46(.) or 44(,) or 45(-)
                    {
                        if(key==46||key==44)
                        {
                            if (text.find(".")== std::string::npos)
                            {
                                text += ".";
                                
                            }
                        }
                        else if (key>=48 && key<=58)
                        {
                            text += std::to_string(key-48);
                            
                        }
                        else if (key == 45)
                        {
                            
                            if(text.size()==0)
                            {
                                text +="-";
                            }
                            
                        }
                    }
                    else
                    {
                        

                        if(IsKeyPressed(KEY_BACKSPACE)&&text.length()>0)
                        {
                            text.pop_back();
                        }
                    }
                }
                if(mode=="text")
                {
                    int key = GetCharPressed();
                    if(key != 0)
                    {
                        
                        text += char(key);
                    }
                    if(IsKeyPressed(KEY_BACKSPACE)&&text.length()>0)
                        {
                            text.pop_back();
                        }
                }
                
            }
            else
            {
                DrawRectangle(xloc,yloc,width,height,BLACK);
            }
            
            DrawRectangle(xloc+2,yloc+2,width-4,height-4,LIGHTGRAY);

            Vector2 sz = MeasureTextEx(fontDefault,text.c_str(),64,1);
            DrawTextEx(fontDefault,text.c_str(),Vector2({xloc+width/2-sz.x/2,yloc+height/2-sz.y/2}),64,1,BLACK);

            
            
        }

        
};

void downloadCitiesJSON()
{
    CURL *curl;
    FILE *fp;
    CURLcode res;
    std::string url = "https://raw.githubusercontent.com/lutangar/cities.json/master/cities.json";
    std::string outfilename = "cities.json";
    curl = curl_easy_init();                                                                                                                                                                                                                                                           
    if (curl)
    {   
        fp = fopen(outfilename.c_str(),"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
    }  
}

void DrawProgressBar(int xloc, int yloc,int width, int height,int max, int current,float fontSize)
{
    DrawRectangle(xloc,yloc,width,height,BLACK);
    
    if(max>0)
    {
        DrawRectangle(xloc,yloc,int((current/float(max))*width),height,GREEN);
    }
    
    std::string aaaa = std::to_string(current)+"/"+std::to_string(max);
    Vector2 sz = MeasureTextEx(fontDefault,aaaa.c_str(),fontSize,1);
    
    DrawTextEx(fontDefault,aaaa.c_str(),Vector2({xloc+width/2-sz.x/2,yloc+height/2-sz.y/2}),fontSize,1,WHITE);
}

void GetCountryCodesFromJson(nlohmann::json data, std::vector<std::string>* country_codes,int* max,int* current)
{
    std::sort(data.begin(), data.end(),[](const nlohmann::json &a, const nlohmann::json &b){return a["country"] < b["country"];});
    country_codes->clear();
    
    *max = int(data.size());
    for (nlohmann::json city : data)
        {
            *current=*current+1;
            if (std::find(country_codes->begin(),country_codes->end(),city["country"] ) == country_codes->end())
            {
                country_codes->push_back(city["country"]);
            }
            
            
        }
        
}
void GetCitiesFromSelectedCountry(nlohmann::json data, std::vector<std::string>* citiesVector, std::string selectedCountryCode,int* max,int* current)
{
    *max = int(data.size());
    for (nlohmann::json i : data)
        {
            *current=*current+1;
            if(i["country"]==selectedCountryCode)
            {
                citiesVector->push_back(i["name"]);
            }
        }

}






nlohmann::json openJsonAndPraseIt(std::string path) //used for cities.json
{
std::ifstream file(path);
nlohmann::json data = nlohmann::ordered_json::parse(file);
file.close();
std::sort(data.begin(), data.end(),[](const nlohmann::json &a, const nlohmann::json &b){return a["name"] < b["name"];});
return data;
}

inline bool doesFileExist (const std::string& name) { //https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exists-using-standard-c-c11-14-17-c
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0);}



int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    

    std::string path = "cities.json";
    nlohmann::json data ;
    std::vector<std::string> country_codes;
    std::vector<std::string> citiesVector;

    InitWindow(screenWidth, screenHeight, "WeatherApp");
    
    fontDefault = LoadFontEx("DejaVuSans.ttf", 32, 0, 6253);

    SetTargetFPS(60); 

    //std::string state = "start";
    std::string state = "savedStart";

    boost::thread t;

    int max=0;          //used to draw progres bar
    int current=0;      //used to draw progres bar

    int row = 0;
    int column = 0;
    Button buttonToEdit;
    InputBox inputBoxToEdit;
    std::vector<Button> buttonsVector;      //vector used to store all buttons
    std::vector<InputBox> inputBoxVector;   //vector used to store all input boxes

    std::string selectedCountryCode = "";

    int currentPage=0;

    std::string selectedCity = "";


    nlohmann::json weatherData;

    std::string lat, lng;

    std::vector<double> temperatures;
    std::vector<std::string> times;
    std::vector<int> humidity;
    std::vector<double> precipitation;
    std::vector<double> windspeed;
    std::vector<double> surfacePressure;

    int horizontal;
    double mintemp;
    double maxtemp;

    std::string chartType = "Temperature";

    auto now = std::chrono::system_clock::now();
    std::string ymdBeforeString;
    std::string ymdString;
    cpr::Response r;

    bool isHistorical = false;

    int maxSavedLocations = 10;
    int SavedLocationsShowing = 1;
    std::fstream savedLocationsFile;
    nlohmann::json savedLocationsJson;
    bool savedAllowed = true;
    
    bool isJSONCitiesLoaded = false;

    if(!doesFileExist("cities.json"))
    {
        downloadCitiesJSON();
    }

    


    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
            mouseX = GetMouseX();
            mouseY = GetMouseY();
            ClearBackground(RAYWHITE);
            if(state == "savedStart")
            {
                savedLocationsFile.open("savedLocations.json",std::fstream::in | std::fstream::out |std::fstream::app);
                savedLocationsFile.seekg(0, std::ios::end);
                
                try
                {
                    if (savedLocationsFile.tellg() == 0)
                    {   //flag
                        savedLocationsFile.seekg(0, std::ios::beg);
                        for (int i =0;i<maxSavedLocations;i++)
                        {
                            savedLocationsJson[i]["name"]= "empty";
                            savedLocationsJson[i]["lat"]= "";
                            savedLocationsJson[i]["lng"]= "";
                        }

                        savedLocationsFile<<savedLocationsJson;
                    }
                    else
                    {
                        savedLocationsFile.seekg(0, std::ios::beg);
                        savedLocationsJson = nlohmann::json::parse(savedLocationsFile); //here is where is parse error if file is fucked up
                    }
                }
                
                catch(nlohmann::json::parse_error& ex)
                {
                    state = "openingSavedLocationsJsonErrorButtons";
                }
                savedLocationsFile.close();

                for (int i =0;i<maxSavedLocations;i++)
                {
                    if(savedLocationsJson[i]["name"] != "empty")
                    {
                        SavedLocationsShowing = i+1;
                    }
                }
                state = "start";
            }
            if(state == "start")
            {
                inputBoxVector.clear();
                buttonsVector.clear();
                buttonToEdit.width = 500;
                buttonToEdit.height = 60;
                buttonToEdit.text = "Take data from json";
                buttonToEdit.yloc = 600;
                buttonToEdit.xloc = 100;
                buttonToEdit.color = BLUE;
                buttonToEdit.fontSize = 20;


                buttonsVector.push_back(buttonToEdit);

                buttonToEdit.text = "input geographic coordinate manualy";
                buttonToEdit.fontSize = 20;
                buttonToEdit.xloc = 680;
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.yloc = 500;
                buttonToEdit.xloc=100;
                buttonToEdit.text = "saved";
                if(savedAllowed)
                {
                    buttonToEdit.color = BLUE;
                }
                else
                {
                    buttonToEdit.color = GRAY;
                }
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.color = BLUE;

                buttonToEdit.xloc = 680;
                buttonToEdit.text = "Download JSON";
                buttonsVector.push_back(buttonToEdit);

                state = "selectMode";
            }
            if(state == "selectMode")
            {
                for (Button i : buttonsVector)
                {
                    i.Draw();
                    if( i.IsClicked())
                    {
                        if (i.text =="Take data from json")
                        {
                            state = "startJSON";
                        }
                        if (i.text == "input geographic coordinate manualy")
                        {
                            buttonsVector.clear();
                            inputBoxToEdit.xloc = 400;
                            inputBoxToEdit.yloc = 185;
                            inputBoxToEdit.width = 600;
                            inputBoxToEdit.height = 150;
                            inputBoxToEdit.text = "";
                            inputBoxToEdit.isSelected = false;
                            inputBoxVector.push_back(inputBoxToEdit);
                            inputBoxToEdit.yloc = 420;
                            inputBoxVector.push_back(inputBoxToEdit);

                            buttonToEdit.xloc = 10;
                            buttonToEdit.yloc = 10;
                            buttonToEdit.width = 500;
                            buttonToEdit.height = 60;
                            buttonToEdit.text = "back to selection";
                            buttonsVector.push_back(buttonToEdit);
                            buttonToEdit.xloc = 770;
                            buttonToEdit.yloc = 650;
                            buttonToEdit.text = "confirm";
                            buttonsVector.push_back(buttonToEdit);
                            state = "manualCoordinates";
                        }
                        if( i.text == "saved" && savedAllowed)
                        {
                            buttonsVector.clear();
                            state = "savedButtons";
                        }
                        if(i.text == "Download JSON")
                        {
                            downloadCitiesJSON();
                        }
                    }
                    
                }
            }
            if(state == "savedButtons")
            {
                try
                {
                    for (int i =0;i<maxSavedLocations;i++)
                    {
                        if (SavedLocationsShowing > i)
                        {

                            buttonToEdit.fontSize = 20;
                            buttonToEdit.width = 500;
                            buttonToEdit.height = 50;
                            buttonToEdit.xloc = 100;
                            buttonToEdit.yloc = 100 + i*50;
                            
                            buttonToEdit.text = savedLocationsJson[i]["name"];
                            if(buttonToEdit.text == "empty")
                            {
                                buttonToEdit.text = buttonToEdit.text + std::to_string(i+1);
                                buttonToEdit.color = GRAY;
                            }
                            else
                            {
                                buttonToEdit.color = BLUE;
                            }
                            buttonsVector.push_back(buttonToEdit);
                            buttonToEdit.width = 100;
                            buttonToEdit.height = 50;
                            buttonToEdit.xloc = 600;
                            buttonToEdit.yloc = 100 + i*50;
                            buttonToEdit.text = "edit"+ std::to_string(i+1);
                            buttonToEdit.color = GREEN;
                            buttonsVector.push_back(buttonToEdit);
                            buttonToEdit.width = 100;
                            buttonToEdit.height = 50;
                            buttonToEdit.xloc = 700;
                            buttonToEdit.yloc = 100 + i*50;
                            buttonToEdit.text = "delete"+ std::to_string(i+1);
                            buttonToEdit.color = RED;
                            buttonsVector.push_back(buttonToEdit);
                            
                        }
                        if (SavedLocationsShowing == i)
                        {
                            buttonToEdit.width = 500;
                            buttonToEdit.height = 50;
                            buttonToEdit.xloc = 200;
                            buttonToEdit.yloc = 100 + i*50;
                            buttonToEdit.text = "add empty";
                            buttonToEdit.color = GREEN;
                            buttonsVector.push_back(buttonToEdit);
                        }
                    }
                    buttonToEdit.xloc = 0;
                    buttonToEdit.yloc = 0;
                    buttonToEdit.width = 500;
                    buttonToEdit.height = 50;
                    buttonToEdit.text = "go back";
                    buttonToEdit.color = BLUE;
                    buttonsVector.push_back(buttonToEdit);
                    
                    state = "saved";
                }
                catch(...)
                {
                    state = "openingSavedLocationsJsonErrorButtons";
                }
                
            }
            if(state == "saved")
            {
                    for (Button i : buttonsVector)
                    {
                        i.Draw();
                        if(i.IsClicked())
                        {
                            if(i.text.starts_with("edit"))
                            {
                                buttonsVector.clear();
                                inputBoxVector.clear();

                                buttonToEdit.xloc = 390;
                                buttonToEdit.yloc = 620;
                                buttonToEdit.width = 500;
                                buttonToEdit.height = 100;
                                buttonToEdit.text = "confirm";
                                buttonToEdit.color = BLUE;
                                buttonsVector.push_back(buttonToEdit);
                                
                                inputBoxToEdit.xloc = 390;
                                inputBoxToEdit.yloc = 100;
                                inputBoxToEdit.height = 100;
                                inputBoxToEdit.width = 500;
                                inputBoxToEdit.mode = "text";
                                inputBoxVector.push_back(inputBoxToEdit);

                                inputBoxToEdit.yloc = 300;
                                inputBoxToEdit.mode = "numbers";
                                inputBoxVector.push_back(inputBoxToEdit);
                                inputBoxToEdit.yloc = 500;
                                inputBoxToEdit.mode = "numbers";
                                inputBoxVector.push_back(inputBoxToEdit);

                                state = "editSaved";
                                

                                currentPage = i.text[4] - '1';  //reusing curretPage
                                
                            }
                            if(i.text.starts_with("delete"))
                            {
                                
                                savedLocationsJson[i.text[6] - '1']["name"]= "empty";
                                savedLocationsJson[i.text[6] - '1']["lat"]= "";
                                savedLocationsJson[i.text[6] - '1']["lng"]= "";
                                
                                
                                savedLocationsFile.open("savedLocations.json",std::fstream::in | std::fstream::out |std::fstream::trunc);
                                savedLocationsFile<<savedLocationsJson;
                                savedLocationsFile.close();
                                
                                
                                buttonsVector.clear();
                                state = "savedButtons";
                            }
                            if(i.text == "go back")
                            {
                                buttonsVector.clear();
                                state = "start";
                            }
                            if(i.text == "add empty")
                            {
                                if(SavedLocationsShowing<maxSavedLocations)
                                {
                                SavedLocationsShowing++;
                                buttonsVector.clear();
                                state = "savedButtons";
                                }
                            }
                            for (int j=0;j<maxSavedLocations;j++)
                            {
                                if(savedLocationsJson[j]["name"] == i.text)
                                {
                                    lat = savedLocationsJson[j]["lat"];
                                    lng = savedLocationsJson[j]["lng"];
                                    state = "sendRequest";
                                }
                            }
                        }
                    }
                    
            }
            if(state == "editSaved")
            {
                DrawTextEx(fontDefault,"name:",Vector2({280,125}),40,1,BLACK);
                DrawTextEx(fontDefault,"Lat:",Vector2({280,325}),40,1,BLACK);
                DrawTextEx(fontDefault,"Lng:",Vector2({280,525}),40,1,BLACK);
                for (InputBox &i : inputBoxVector)
                    {
                        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                        {
                            if((mouseX>i.xloc && mouseX<(i.xloc+i.width))&&(mouseY>i.yloc && mouseY<(i.yloc+i.height)))  
                            {
                                i.isSelected = true;
                                
                            } 
                            else
                            {
                                i.isSelected = false;
                            }
                        }
                        i.Draw();
                    }
                for (Button &i : buttonsVector)
                {
                    i.Draw();
                    if(i.IsClicked())
                    {
                        if(i.text == "confirm")
                        {
                            savedLocationsJson[currentPage]["name"]= inputBoxVector[0].text;
                            savedLocationsJson[currentPage]["lat"]= inputBoxVector[1].text;
                            savedLocationsJson[currentPage]["lng"]= inputBoxVector[2].text;
                            currentPage =0; //restart to default value
                            buttonsVector.clear();
                            inputBoxVector.clear();
                            state = "savedButtons";

                            savedLocationsFile.open("savedLocations.json",std::fstream::in | std::fstream::out |std::fstream::trunc);
                            savedLocationsFile<<savedLocationsJson;
                            savedLocationsFile.close();
                        }
                    }
                }
            }
            if(state == "manualCoordinates")
            {
                for (InputBox &i : inputBoxVector)  //by adding & before i I'm not making copy byt just referencing thing in vector?
                {
                    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        if((mouseX>i.xloc && mouseX<(i.xloc+i.width))&&(mouseY>i.yloc && mouseY<(i.yloc+i.height)))  
                        {
                            i.isSelected = true;
                            
                        } 
                        else
                        {
                            i.isSelected = false;
                        }
                    }
                    i.Draw();
                }
                for (Button i: buttonsVector)
                {
                    i.Draw();
                    if( i.IsClicked())
                    {
                        if (i.text =="back to selection")
                        {
                            
                            
                            state = "start";
                        }
                        if (i.text =="confirm")
                        {
                            if(inputBoxVector[0].text.size()!=0 && inputBoxVector[1].text.size()!=0)
                            {
                                lat = inputBoxVector[0].text;
                                lng = inputBoxVector[1].text;
                                state = "sendRequest";
                            }
                        }

                    }
                }
                DrawTextEx(fontDefault,"lat:",Vector2({float(inputBoxVector[0].xloc-100),float(inputBoxVector[0].yloc+50)}),60,1,BLACK);
                DrawTextEx(fontDefault,"lng:",Vector2({float(inputBoxVector[1].xloc-100),float(inputBoxVector[1].yloc+50)}),60,1,BLACK);
                

            }
            if(state == "startJSON") //state start is first state that will be drawn on screen, it will tell user that file cities.json is being loaded
            {
                buttonsVector.clear();
                DrawText("file cities.json is being loaded, if this text will not disapear in 1 minute restart aplication", 190, 200, 20, LIGHTGRAY);
                DrawText("make sure you have file cities.json in same directory as executable file", 190, 230, 20, LIGHTGRAY);
                state ="loadJSON"; 
            }
            if(state == "loadJSON")
                {
                    if(isJSONCitiesLoaded == false)   
                    {
                        data = openJsonAndPraseIt(boost::ref(path)); //nlohmann::json has undefined behavior when you write data to refrence
                        //can't prase json in thread because of it I think, or mayby I'm just stupid
                        state = "getCountryCodesStart";
                    }
                    else
                    {
                        state ="sellectCountryStart";
                    }
                }
            if(state == "getCountryCodesStart")
                {
                    
                    current = 0;
                    t = boost::thread(boost::bind(&GetCountryCodesFromJson,boost::ref(data),&country_codes,&max,&current));
                    state = "getCountryCodesLoading";
                }
            if(state == "getCountryCodesLoading")
            {
                DrawProgressBar(440,260,400,200,max,current,50);
                    if(t.timed_join(boost::posix_time::seconds(0))) //checks if thread finished
                    {
                        t.join();
                        isJSONCitiesLoaded = true;
                        state = "sellectCountryStart";
                    }
                    
            }
            if(state == "sellectCountryStart")
            {
                std::sort(data.begin(), data.end(),[](const nlohmann::json &a, const nlohmann::json &b){return a["name"] < b["name"];});
                buttonsVector.clear();
                row = 3; 
                column =0; 
                buttonToEdit.height =38;
                buttonToEdit.width =38;
                
                
                buttonToEdit.color = BLUE;
                
                for(std::string i : country_codes)
                {
                    buttonToEdit.xloc =2 + column*40;
                    buttonToEdit.yloc =2 + row *40;
                    buttonToEdit.text = i;
                    buttonToEdit.fontSize = 20;
                    buttonsVector.push_back(buttonToEdit);
                    column++;
                    if(column >=32)
                    {
                        column =0;
                        row++;
                    }
                }
                buttonToEdit.xloc = 0;
                buttonToEdit.yloc = 0;
                buttonToEdit.height =60;
                buttonToEdit.width =200;
                buttonToEdit.text = "back to selection";
                buttonToEdit.fontSize = 20;
                buttonsVector.push_back(buttonToEdit);
                state = "sellectCountryButtonScreen";
            }
            if(state == "sellectCountryButtonScreen")
            {
                selectedCountryCode = "";
                DrawText("Select code of country from which you want to see list of cities",150,80,30,BLUE);
                //DrawRectangle(40,40,38,38,BLUE);
                for(Button i : buttonsVector)
                {
                    i.Draw();
                    if(i.IsClicked()&& i.text != "back to selection") 
                    {
                        selectedCountryCode = i.text;
                    }
                    else if (i.IsClicked())
                    {
                        state = "start";
                    }
                }
                if(selectedCountryCode != "")
                {
                    state = "GetCitiesFromCountryStart";
                }
            }
            if(state == "GetCitiesFromCountryStart")
            {
                currentPage = 0;
                citiesVector.clear();
                buttonsVector.clear(); //destroying all buttons and retruning the memory?
                current = 0;
                t = boost::thread(boost::bind(&GetCitiesFromSelectedCountry,boost::ref(data),&citiesVector,boost::ref(selectedCountryCode),&max,&current));
                state = "GetCitiesFromCountryThread";
            }
            if(state == "GetCitiesFromCountryThread")
            {

                DrawProgressBar(440,260,400,200,max,current,50);
                if(t.timed_join(boost::posix_time::seconds(0))) //checks if thread finished
                {
                    
                    t.join();
                    state = "sellectCityButtonScreenCreateButtons";
                }
            }
            if (state == "sellectCityButtonScreenCreateButtons")
            {
                
                buttonsVector.clear();
                buttonToEdit.xloc = 30;
                buttonToEdit.yloc = 590;    
                buttonToEdit.height = 38;
                buttonToEdit.width = 400;
                buttonToEdit.text = "previous page";
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.yloc = 100;
                buttonToEdit.text = "back to country codes";
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.yloc = 590;
                buttonToEdit.xloc = 850;
                buttonToEdit.text = "next page";
                buttonsVector.push_back(buttonToEdit);
                for (int j = 0; j < 3;j++)
                {
                    for (int i =0;i<10;i++)
                    {
                        
                        

                        if(i+j*10+currentPage*30 < citiesVector.size())
                        {
                        
                        buttonToEdit.xloc = 30 + j*410;
                        buttonToEdit.yloc = 150 + i*40;
                        buttonToEdit.height = 38;
                        buttonToEdit.width = 400;
                        buttonToEdit.text = citiesVector[i +currentPage*30 +j*10];
                        buttonToEdit.fontSize = 20;
                        buttonToEdit.color = BLUE;
                        buttonsVector.push_back(buttonToEdit);
                        }
                    }
                }
                
                state = "sellectCityButtonScreen";
            }
            if (state == "sellectCityButtonScreen")
            {
                for (Button i : buttonsVector)
                {
                    i.Draw();
                    if(i.IsClicked() && i.text == "next page" && (currentPage+1)*30< citiesVector.size())
                    {
                        
                        currentPage +=1;
                        state = "sellectCityButtonScreenCreateButtons";
                        
                        
                    }
                    if(i.IsClicked() && i.text == "previous page" && currentPage>0)
                    {
                        currentPage -=1;
                        state = "sellectCityButtonScreenCreateButtons";
                    } 
                    if(i.IsClicked() && i.text == "back to country codes")
                    {
                        
                        state = "sellectCountryStart";
                    }
                    if (i.IsClicked() && i.text != "next page" && i.text != "previous page" && i.text != "back to country codes")
                    {
                        selectedCity = i.text;
                        state = "getLatAndLng";
                    }
                }
                
            }
            if (state == "getLatAndLng")
            {
                for (nlohmann::json i : data)
                {
                    
                    if(i["name"]==selectedCity)
                    {
                        lat = std::string(i["lat"]);
                        lng = std::string(i["lng"]);
                        
                    }

                }
                //std::cout<<lat<<std::endl;
                //std::cout<<lng<<std::endl;

                state = "sendRequest";
            }
            
            if (state == "sendRequest")
            {   inputBoxVector.clear();
                buttonsVector.clear();
                r = cpr::Get(cpr::Url{"https://api.open-meteo.com/v1/forecast?latitude="+lat+"&longitude="+lng+"&hourly=temperature_2m,relativehumidity_2m,precipitation,windspeed_10m,surface_pressure&models=best_match"});
                
                
                if(std::to_string(r.status_code).compare(0,1,"2") != 0)
                {
                    state = "errorStatusCode";
                    buttonsVector.clear();
                    buttonToEdit.text = "back to start";
                    buttonToEdit.xloc = 100;
                    buttonToEdit.yloc = 500;
                    buttonToEdit.height = 100;
                    buttonToEdit.width = 300;
                    buttonToEdit.color = BLUE;
                    buttonToEdit.fontSize = 30;
                    buttonsVector.push_back(buttonToEdit);
                }
                else
                {
                
                weatherData= nlohmann::json::parse(r.text);
                state = "extractData";
                }
                
                
            }
            if (state == "extractData")
            {
                
                temperatures.clear();
                times.clear();
                humidity.clear();
                precipitation.clear();
                windspeed.clear();
                surfacePressure.clear();
                for (nlohmann::json i : weatherData["hourly"]["temperature_2m"])
                {
                    temperatures.push_back(i);
                }
                for (nlohmann::json i : weatherData["hourly"]["time"])
                {
                    times.push_back(i);
                }
                for(nlohmann::json i : weatherData["hourly"]["relativehumidity_2m"])
                {
                    humidity.push_back(i);
                }
                for(nlohmann::json i : weatherData["hourly"]["precipitation"])
                {
                    precipitation.push_back(i);
                }
                for(nlohmann::json i : weatherData["hourly"]["windspeed_10m"])
                {
                    windspeed.push_back(i);
                }
                for(nlohmann::json i : weatherData["hourly"]["surface_pressure"])
                {
                    surfacePressure.push_back(i);
                }
                /*
                for (int i=0;i<temperatures.size();i++)
                {
                    std::cout<<times[i]<<" "<<temperatures[i]<<std::endl; 
                }
                */
                //std::cout<<temperatures.size()<<std::endl;
                

                maxtemp = *std::max_element(temperatures.begin(),temperatures.end());
                mintemp = *std::min_element(temperatures.begin(),temperatures.end());
                horizontal =int( (maxtemp-std::fmod(maxtemp,1)+1) - (mintemp - std::fmod(mintemp,1)-1)) ;//amout of horizontal lines on quick preview spaced 1 celcious apart
                
                
                
                //std::cout<<"horizontal "<<horizontal<<std::endl;

                //std::cout<<"maxtemp "<<maxtemp<<std::endl;
                //std::cout<<"mintemp "<<mintemp<<std::endl;

                //std::cout<<"test1: "<<(maxtemp-std::fmod(maxtemp,1)+1)<<std::endl;
                //std::cout<<"test2: "<<(mintemp - std::fmod(mintemp,1)-1)<<std::endl;

                buttonToEdit.width = 100;
                buttonToEdit.height = 40;
                buttonToEdit.text = "Temperature";
                buttonToEdit.xloc = 1100;
                buttonToEdit.yloc = 100;
                buttonToEdit.fontSize = 16;
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.yloc = 150;
                buttonToEdit.fontSize = 11;
                buttonToEdit.text = "Relative Humidity";
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.yloc = 200;
                buttonToEdit.fontSize = 15;
                buttonToEdit.text = "Precipitation";
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.yloc = 250;
                buttonToEdit.fontSize = 16;
                buttonToEdit.text = "Wind speed";
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.yloc = 300;
                buttonToEdit.fontSize = 11;
                buttonToEdit.text = "Surface pressure";
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.yloc = 350;
                buttonToEdit.fontSize = 11;
                buttonToEdit.text = "historicalWeather";
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.yloc = 400;
                buttonToEdit.fontSize = 11;
                buttonToEdit.text = "currentWeather";
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.yloc = 0;
                buttonToEdit.xloc = 0;
                buttonToEdit.text = "back to start";
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.yloc = 0;
                buttonToEdit.xloc = 100;
                buttonToEdit.text = "add to saved";
                buttonsVector.push_back(buttonToEdit);
                state = "showDataMenu";
            }
            if (state == "showDataMenu")
            {
                double current ;
                DrawRectangle(200,100,880,520,Color({ 220, 220, 220, 255 }));
                std::string title = "lat:"+lat+" lng:"+lng;
                DrawTextEx(fontDefault,(title).c_str(),Vector2({640-MeasureTextEx(fontDefault,title.c_str(),50,1).x/2,50}),50,1,BLUE);
                
                DrawTextEx(fontDefault,chartType.c_str(),Vector2({640-MeasureTextEx(fontDefault,chartType.c_str(),50,1).x/2,100}),50,1,BLUE);
                if(chartType == "Temperature")
                {
                     current = temperatures[0];
                    
                    //std::cout<<"i:    "<<(*std::min_element(temperatures.begin(),temperatures.end()) - std::fmod(*std::min_element(temperatures.begin(),temperatures.end()) ,1)+1)<<std::endl;
                    //std::cout<<"imax: "<<((*std::max_element(temperatures.begin(),temperatures.end()) - std::fmod(*std::max_element(temperatures.begin(),temperatures.end()) ,1)+1)-(*std::min_element(temperatures.begin(),temperatures.end()) - std::fmod(*std::min_element(temperatures.begin(),temperatures.end()) ,1)+1))<<std::endl;
                    int maxElement = int( *std::max_element(temperatures.begin(),temperatures.end()) - std::fmod(*std::max_element(temperatures.begin(),temperatures.end()),1.0)+1);
                    int minElement = int(*std::min_element(temperatures.begin(),temperatures.end()) - std::fmod(*std::min_element(temperatures.begin(),temperatures.end()) ,1)-1);
                    int lineCount = int((*std::max_element(temperatures.begin(),temperatures.end()) - std::fmod(*std::max_element(temperatures.begin(),temperatures.end()) ,1)+1)-(*std::min_element(temperatures.begin(),temperatures.end()) - std::fmod(*std::min_element(temperatures.begin(),temperatures.end()) ,1)-1));
                    
                    for(int i =0;i<=lineCount;i++)
                    {
                        
                        if(minElement+i== 0)
                        {
                        DrawLineEx(Vector2({200,float(500-(300.0/lineCount)*i) }),Vector2({1080,float(500-(300.0/lineCount)*i) }),1,BLUE);
                        }
                        else
                        {
                        DrawLineEx(Vector2({200,float(500-(300.0/lineCount)*i) }),Vector2({1080,float(500-(300.0/lineCount)*i) }),1,Color({ 160, 160, 160, 255 }));
                        }
                        DrawTextEx(fontDefault,(std::to_string(int(minElement) + i)+std::string(weatherData["hourly_units"]["temperature_2m"])).c_str(),Vector2({150.0,float(500-(300/lineCount)*i-9)}),16,1,BLACK);
                    }
                    
                    for(int i =0 ; i < temperatures.size();i++)
                    {
                        
                        DrawLineEx(Vector2({float((i)*5.238+200),float(500-300*((current -minElement)/( maxElement - minElement)))}),Vector2({float((i+1)*5.238+200),float(500-300*((temperatures[i] -minElement)/( maxElement - minElement)))}),2,RED);
                        current = temperatures[i];
                    }
                    
                }
                else if (chartType == "Relative Humidity")
                {
                    current = humidity[0];
                    for(int i =0;i<=5;i++)
                    {
                        DrawLineEx(Vector2({200,float(500-60*i)}),Vector2({1080,float(500-60*i)}),1,Color({ 160, 160, 160, 255 }));
                        DrawTextEx(fontDefault,(std::to_string(i*20)+ std::string(weatherData["hourly_units"]["relativehumidity_2m"])).c_str(),Vector2({160.0,float(500-60*i-9)}),16,1,BLACK);
                    }
                    for(int i = 1;i<humidity.size();i++)
                    {
                        DrawLineEx(Vector2({float((i-1)*5.238+200),float(500-current*3)}),Vector2({float((i)*5.238+200),float(500-humidity[i]*3)}),2,RED);
                        current = humidity[i];
                    }
                }
                else if (chartType == "Precipitation")
                {
                    current = precipitation[0];
                    //std::cout<<*std::max_element(precipitation.begin(),precipitation.end())<<std::endl;
                    for(int i =0;i<=*std::max_element(precipitation.begin(),precipitation.end()) - std::fmod(*std::max_element(precipitation.begin(),precipitation.end()) ,1)+1;i++)
                    {
                        //std::cout<< (*std::max_element(precipitation.begin(),precipitation.end()) - std::fmod(*std::max_element(precipitation.begin(),precipitation.end()) ,1)+1) <<std::endl;
                        //std::cout<<"aaa"<<i<<std::endl;
                        //std::cout<<float( *std::max_element(precipitation.begin(),precipitation.end()) - std::fmod(*std::max_element(precipitation.begin(),precipitation.end()),1.0)+1)<<std::endl;
                        DrawLineEx(Vector2({200,float(500-(300/( *std::max_element(precipitation.begin(),precipitation.end()) - std::fmod(*std::max_element(precipitation.begin(),precipitation.end()),1.0)+1))*i) }),Vector2({1080,float(500-(300/( *std::max_element(precipitation.begin(),precipitation.end()) - std::fmod(*std::max_element(precipitation.begin(),precipitation.end()),1.0)+1))*i) }),1,Color({ 160, 160, 160, 255 }));
                        DrawTextEx(fontDefault,(std::to_string(i)+std::string(weatherData["hourly_units"]["precipitation"])).c_str(),Vector2({160.0,float(500-(300/( *std::max_element(precipitation.begin(),precipitation.end()) - std::fmod(*std::max_element(precipitation.begin(),precipitation.end()),1.0)+1))*i-9)}),16,1,BLACK);
                    }
                    for(int i =0 ; i < precipitation.size();i++)
                    {
                        DrawLineEx(Vector2({float((i)*5.238+200),float(500-current*300/( *std::max_element(precipitation.begin(),precipitation.end()) - std::fmod(*std::max_element(precipitation.begin(),precipitation.end()),1.0)+1))}),Vector2({float((i+1)*5.238+200),float(500-precipitation[i]*300/( *std::max_element(precipitation.begin(),precipitation.end()) - std::fmod(*std::max_element(precipitation.begin(),precipitation.end()),1.0)+1))}),2,RED);
                        current = precipitation[i];
                    }
                }
                else if (chartType == "Wind speed")
                {
                    
                    current = windspeed[0];
                    
                    for(int i =0;i<=*std::max_element(windspeed.begin(),windspeed.end()) - std::fmod(*std::max_element(windspeed.begin(),windspeed.end()) ,1)+1;i++)
                    {
                        
                        DrawLineEx(Vector2({200,float(500-(300/( *std::max_element(windspeed.begin(),windspeed.end()) - std::fmod(*std::max_element(windspeed.begin(),windspeed.end()),1.0)+1))*i) }),Vector2({1080,float(500-(300/( *std::max_element(windspeed.begin(),windspeed.end()) - std::fmod(*std::max_element(windspeed.begin(),windspeed.end()),1.0)+1))*i) }),1,Color({ 160, 160, 160, 255 }));
                        DrawTextEx(fontDefault,(std::to_string(i)+std::string(weatherData["hourly_units"]["windspeed_10m"])).c_str(),Vector2({140.0,float(500-(300/( *std::max_element(windspeed.begin(),windspeed.end()) - std::fmod(*std::max_element(windspeed.begin(),windspeed.end()),1.0)+1))*i-9)}),16,1,BLACK);
                    }
                    for(int i =0 ; i < windspeed.size();i++)
                    {
                        DrawLineEx(Vector2({float((i)*5.238+200),float(500-current*300/( *std::max_element(windspeed.begin(),windspeed.end()) - std::fmod(*std::max_element(windspeed.begin(),windspeed.end()),1.0)+1))}),Vector2({float((i+1)*5.238+200),float(500-windspeed[i]*300/( *std::max_element(windspeed.begin(),windspeed.end()) - std::fmod(*std::max_element(windspeed.begin(),windspeed.end()),1.0)+1))}),2,RED);
                        current = windspeed[i];
                    }
                }
                else if (chartType == "Surface pressure")
                {
                    current = surfacePressure[0];
                    
                    //std::cout<<"i:    "<<(*std::min_element(surfacePressure.begin(),surfacePressure.end()) - std::fmod(*std::min_element(surfacePressure.begin(),surfacePressure.end()) ,1)+1)<<std::endl;
                    //std::cout<<"imax: "<<((*std::max_element(surfacePressure.begin(),surfacePressure.end()) - std::fmod(*std::max_element(surfacePressure.begin(),surfacePressure.end()) ,1)+1)-(*std::min_element(surfacePressure.begin(),surfacePressure.end()) - std::fmod(*std::min_element(surfacePressure.begin(),surfacePressure.end()) ,1)+1))<<std::endl;
                    int maxElement = int( *std::max_element(surfacePressure.begin(),surfacePressure.end()) - std::fmod(*std::max_element(surfacePressure.begin(),surfacePressure.end()),1.0)+1);
                    int minElement = int(*std::min_element(surfacePressure.begin(),surfacePressure.end()) - std::fmod(*std::min_element(surfacePressure.begin(),surfacePressure.end()) ,1));
                    int lineCount = int((*std::max_element(surfacePressure.begin(),surfacePressure.end()) - std::fmod(*std::max_element(surfacePressure.begin(),surfacePressure.end()) ,1)+1)-(*std::min_element(surfacePressure.begin(),surfacePressure.end()) - std::fmod(*std::min_element(surfacePressure.begin(),surfacePressure.end()) ,1)));
                    for(int i =0;i<=lineCount;i++)
                    {
                        DrawLineEx(Vector2({200,float(500-(300.0/lineCount)*i) }),Vector2({1080,float(500-(300.0/lineCount)*i) }),1,Color({ 160, 160, 160, 255 }));
                        DrawTextEx(fontDefault,(std::to_string(int(*std::min_element(surfacePressure.begin(),surfacePressure.end()) - std::fmod(*std::min_element(surfacePressure.begin(),surfacePressure.end()) ,1)) + i)+std::string(weatherData["hourly_units"]["surface_pressure"])).c_str(),Vector2({130.0,float(500-(300/((*std::max_element(surfacePressure.begin(),surfacePressure.end()) - std::fmod(*std::max_element(surfacePressure.begin(),surfacePressure.end()) ,1)+1)-(*std::min_element(surfacePressure.begin(),surfacePressure.end()) - std::fmod(*std::min_element(surfacePressure.begin(),surfacePressure.end()) ,1))))*i-9)}),16,1,BLACK);
                    }
                    
                    for(int i =0 ; i < surfacePressure.size();i++)
                    {
                        
                        DrawLineEx(Vector2({float((i)*5.238+200),float(500-300*((current -minElement)/( maxElement - minElement)))}),Vector2({float((i+1)*5.238+200),float(500-300*((surfacePressure[i] -minElement)/( maxElement - minElement)))}),2,RED);
                        current = surfacePressure[i];
                    }
                }

                for(int i =0;i<7;i++)
                {
                    DrawTextEx(fontDefault,times[i*24].substr(0, 10).c_str(),Vector2({float(200+880/7*i),520.0}),16,1,BLACK);
                    DrawLineEx(Vector2({float(200+880/7*i),200}),Vector2({float(200+880/7*i),525}),1,Color({ 160, 160, 160, 128 }));
                    //std::cout<<times[i*24].substr(0, 10)<<std::endl;
                    
                    
                }
                if (!isHistorical)
                {
                    
                    DrawLineEx(Vector2({float(200+880.0/7.0/24.0*boost::posix_time::second_clock::local_time().time_of_day().hours()+880.0/7.0/24.0/60.0*boost::posix_time::second_clock::local_time().time_of_day().minutes()),200}),Vector2({float(200+880.0/7.0/24.0*boost::posix_time::second_clock::local_time().time_of_day().hours()+880.0/7.0/24.0/60.0*boost::posix_time::second_clock::local_time().time_of_day().minutes()),525}),2,Color({ 76, 172, 108, 128 }));
                }
                
                for (Button i : buttonsVector)
                {
                    i.Draw();

                    if(i.IsClicked())
                    {
                        if(i.text !="historicalWeather" && i.text !="currentWeather" && i.text != "back to start" && i.text != "add to saved")
                            {chartType = i.text;}
                        else
                        {
                            buttonsVector.clear();
                            if(i.text == "historicalWeather")
                            {
                                isHistorical = true;
                                state = "historicalWeather";
                            }
                            if(i.text =="currentWeather")
                            {
                                isHistorical = false;
                                state = "sendRequest";
                            }
                            if(i.text == "back to start")
                            {
                                state = "start";
                                
                            }
                            if(i.text == "add to saved")
                            {
                                
                                

                                bool NotFoundEmpty = true;
                                //TEST DEBUG
                                for (int i =0;i<maxSavedLocations;i++)
                                {
                                    
                                    if(savedLocationsJson[i]["name"]== "empty" && NotFoundEmpty)
                                        {
                                        currentPage = i;
                                        NotFoundEmpty = false;}
                                }
                                if(NotFoundEmpty)
                                {
                                    state = "saved";
                                }
                                else
                                {
                                    buttonsVector.clear();
                                    inputBoxVector.clear();

                                    buttonToEdit.xloc = 390;
                                    buttonToEdit.yloc = 620;
                                    buttonToEdit.width = 500;
                                    buttonToEdit.height = 100;
                                    buttonToEdit.text = "confirm";
                                    buttonToEdit.color = BLUE;
                                    buttonToEdit.fontSize = 20;
                                    buttonsVector.push_back(buttonToEdit);
                                    
                                    inputBoxToEdit.xloc = 390;
                                    inputBoxToEdit.yloc = 100;
                                    inputBoxToEdit.height = 100;
                                    inputBoxToEdit.width = 500;
                                    inputBoxToEdit.mode = "text";
                                    inputBoxToEdit.text = "";
                                    inputBoxVector.push_back(inputBoxToEdit);

                                    inputBoxToEdit.yloc = 300;
                                    inputBoxToEdit.mode = "numbers";
                                    inputBoxToEdit.text = lat;
                                    inputBoxVector.push_back(inputBoxToEdit);
                                    inputBoxToEdit.yloc = 500;
                                    inputBoxToEdit.mode = "numbers";
                                    inputBoxToEdit.text = lng;
                                    inputBoxVector.push_back(inputBoxToEdit);
                                   
                                    if(currentPage>=SavedLocationsShowing)
                                    {SavedLocationsShowing++;}
                                    state = "editSaved";
                                }
                                
                            }
                        }
                    }
                }
                
            }
            if(state == "historicalWeather")
            {
                buttonsVector.clear();
                buttonToEdit.text = "-1 month";
                buttonToEdit.yloc = 500;
                buttonToEdit.xloc = 20;
                buttonToEdit.height = 100;
                buttonToEdit.width = 180;
                buttonToEdit.color = BLUE;
                buttonToEdit.fontSize = 24;
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.text = "-1 week";
                buttonToEdit.xloc = 220;
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.text = "-1 day";
                buttonToEdit.xloc = 420;
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.text = "+1 day";
                buttonToEdit.xloc = 680;
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.text = "+1 week";
                buttonToEdit.xloc = 880;
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.text = "+1 month";
                buttonToEdit.xloc = 1080;
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.text = "submit";
                buttonToEdit.yloc = 610;
                buttonToEdit.xloc = 420;
                buttonToEdit.width = 440;
                buttonsVector.push_back(buttonToEdit);

                
                
                
                
                state = "historicalMenu";
            }
            if(state == "historicalMenu")
            {
                for(Button i : buttonsVector)
                {
                    i.Draw();
                    if(i.IsClicked())
                    {
                        if(i.text=="-1 month")
                        {
                            now = now - std::chrono::months(1);
                        }
                        if(i.text=="-1 week")
                        {
                            now = now - std::chrono::weeks(1);
                        }
                        if(i.text=="-1 day")
                        {
                            now = now - std::chrono::days(1);
                        }
                        if(i.text=="+1 month")
                        {
                            now = now + std::chrono::months(1);
                        }
                        if(i.text=="+1 week")
                        {
                            now = now + std::chrono::weeks(1);
                        }
                        if(i.text=="+1 day")
                        {
                            now = now + std::chrono::days(1);
                        }
                        if(i.text=="submit")
                        {
                            state = "historicalSendRequest";
                        }
                    }
                }
                {
                    std::chrono::year_month_day ymdBefore{std::chrono::floor<std::chrono::days>(now- std::chrono::weeks(1))};
                    std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)-std::chrono::days(1)};
                    
                    ymdBeforeString = std::to_string(static_cast<int>(ymdBefore.year()))+"-";
                    if(std::to_string(static_cast<unsigned>(ymdBefore.month())).length()==1)
                    {
                        ymdBeforeString += "0"+std::to_string(static_cast<unsigned>(ymdBefore.month()))+"-";
                    }
                    else
                    {
                        ymdBeforeString +=std::to_string(static_cast<unsigned>(ymdBefore.month()))+"-";
                    }

                    if(std::to_string(static_cast<unsigned>(ymdBefore.day())).length()==1)
                    {
                        ymdBeforeString += "0"+std::to_string(static_cast<unsigned>(ymdBefore.day()));
                    }
                    else
                    {
                        ymdBeforeString +=std::to_string(static_cast<unsigned>(ymdBefore.day()));
                    }
                    
                    ymdString = std::to_string(static_cast<int>(ymd.year()))+"-";
                    if(std::to_string(static_cast<unsigned>(ymd.month())).length()==1)
                    {
                        ymdString += "0"+std::to_string(static_cast<unsigned>(ymd.month()))+"-";
                    }
                    else
                    {
                        ymdString +=std::to_string(static_cast<unsigned>(ymd.month()))+"-";
                    }

                    if(std::to_string(static_cast<unsigned>(ymd.day())).length()==1)
                    {
                        ymdString += "0"+std::to_string(static_cast<unsigned>(ymd.day()));
                    }
                    else
                    {
                        ymdString +=std::to_string(static_cast<unsigned>(ymd.day()));
                    }

                    
                    DrawTextEx(fontDefault,(ymdBeforeString+" to "+ymdString).c_str(),Vector2({278,100}),64,1,BLACK);
                }
            }
            if(state =="historicalSendRequest")
            {
                inputBoxVector.clear();
                buttonsVector.clear();
                r = cpr::Get(cpr::Url{"https://archive-api.open-meteo.com/v1/era5?latitude="+lat+"&longitude="+lng+"&start_date="+ymdBeforeString+"&end_date="+ymdString+"&hourly=temperature_2m,relativehumidity_2m,surface_pressure,precipitation,windspeed_10m&timezone=auto"});
                weatherData.clear();
                if(std::to_string(r.status_code).compare(0,1,"2") != 0)
                {
                    state = "errorStatusCode";
                    buttonsVector.clear();
                    buttonToEdit.text = "back to start";
                    buttonToEdit.xloc = 100;
                    buttonToEdit.yloc = 500;
                    buttonToEdit.height = 100;
                    buttonToEdit.width = 300;
                    buttonToEdit.color = BLUE;
                    buttonToEdit.fontSize = 30;
                    buttonsVector.push_back(buttonToEdit);
                    
                }
                else
                {
                    weatherData= nlohmann::json::parse(r.text);
                    state = "extractData";
                }
            }
            if (state == "errorStatusCode")
            {
                
                
                if(r.status_code == 0)
                {
                DrawTextEx(fontDefault,"please ensure that you have stable internet connection",Vector2({0,0}),50,1,BLUE);
                DrawTextEx(fontDefault,"HTTP request returned error",Vector2({0,100}),50,1,BLACK);
                DrawTextEx(fontDefault,("error code: "+std::to_string(r.status_code)).c_str(),Vector2({0,200}),50,1,BLACK);
                }
                else
                {
                DrawTextEx(fontDefault,"HTTP request returned error",Vector2({0,0}),50,1,BLACK);
                DrawTextEx(fontDefault,("error code: "+std::to_string(r.status_code)).c_str(),Vector2({0,100}),50,1,BLACK);
                
                
                switch(r.status_code)
                {
                    case 400:
                        DrawTextEx(fontDefault,"Bad Request",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 401:
                        DrawTextEx(fontDefault,"Unauthorized",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 402:
                        DrawTextEx(fontDefault,"Payment Required",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 403:
                        DrawTextEx(fontDefault,"Forbidden",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 404:
                        DrawTextEx(fontDefault,"Not Found",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 405:
                        DrawTextEx(fontDefault,"Method Not Allowed",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 406:
                        DrawTextEx(fontDefault,"Not Acceptable",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 407:
                        DrawTextEx(fontDefault,"Proxy Authentication Required",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 408:
                        DrawTextEx(fontDefault,"Request Timeout",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 409:
                        DrawTextEx(fontDefault,"Conflict",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 410:
                        DrawTextEx(fontDefault,"Gone",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 411:
                        DrawTextEx(fontDefault,"Length Required",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 412:
                        DrawTextEx(fontDefault,"Precondition Failed",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 413:
                        DrawTextEx(fontDefault,"Payload Too Large",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 414:
                        DrawTextEx(fontDefault,"URI Too Long",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 415:
                        DrawTextEx(fontDefault,"Unsupported Media Type",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 416:
                        DrawTextEx(fontDefault,"Range Not Satisfiable",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 417:
                        DrawTextEx(fontDefault,"Expectation Failed",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 418:
                        DrawTextEx(fontDefault,"I'm a teapot",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 421:
                        DrawTextEx(fontDefault,"Misdirected Request",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 422:
                        DrawTextEx(fontDefault,"Unprocessable Entity",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 423:
                        DrawTextEx(fontDefault,"Locked",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 424:
                        DrawTextEx(fontDefault,"Failed Dependency",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 425:
                        DrawTextEx(fontDefault,"Too Early",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 426:
                        DrawTextEx(fontDefault,"Upgrade Required",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 428:
                        DrawTextEx(fontDefault,"Precondition Required",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 429:
                        DrawTextEx(fontDefault,"Too Many Requests",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 431:
                        DrawTextEx(fontDefault,"Request Header Fields Too Large",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 451:
                        DrawTextEx(fontDefault,"Unavailable For Legal Reasons",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 500:
                        DrawTextEx(fontDefault,"Internal Server Error",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 501:
                        DrawTextEx(fontDefault,"Not Implemented",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 502:
                        DrawTextEx(fontDefault,"Bad Gateway",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 503:
                        DrawTextEx(fontDefault,"Service Unavailable",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 504:
                        DrawTextEx(fontDefault,"Gateway Timeout",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 505:
                        DrawTextEx(fontDefault,"HTTP Version Not Supported",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 506:
                        DrawTextEx(fontDefault,"Variant Also Negotiates",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 507:
                        DrawTextEx(fontDefault,"Insufficient Storage",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 508:
                        DrawTextEx(fontDefault,"Loop Detected",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 510:
                        DrawTextEx(fontDefault,"Not Extended",Vector2({0,200}),50,1,BLUE);
                        break;
                    case 511:
                        DrawTextEx(fontDefault,"Network Authentication Required",Vector2({0,200}),50,1,BLUE);
                        break;
                    
                    default:
                        DrawTextEx(fontDefault,"unknown error code, check on internet what it means",Vector2({0,200}),50,1,BLUE);
                        break;

                }
                DrawTextEx(fontDefault,"you can check what all error codes coresponds to at:",Vector2({0,300}),50,1,BLACK);
                DrawTextEx(fontDefault,"https://developer.mozilla.org/en-US/docs/Web/HTTP/Status",Vector2({0,400}),50,1,BLACK);
                }
                for(Button i : buttonsVector)
                {
                    i.Draw();
                    if(i.IsClicked() && i.text == "back to start")
                    {
                        buttonsVector.clear();
                        state = "start";
                    }
                }
            }
            if(state == "openingSavedLocationsJsonErrorButtons")
            {
                buttonsVector.clear();
                buttonToEdit.color = BLUE;
                buttonToEdit.fontSize = 30;
                buttonToEdit.height = 100;
                buttonToEdit.width = 300;
                buttonToEdit.yloc = 580;

                
                buttonToEdit.text = "run without saved";
                buttonToEdit.xloc = 40;
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.text = "try again";
                buttonToEdit.xloc = 490;
                buttonsVector.push_back(buttonToEdit);
                buttonToEdit.text = "close app";
                buttonToEdit.xloc = 940;
                buttonsVector.push_back(buttonToEdit);
                state = "openingSavedLocationsJsonError";
            }
            if(state == "openingSavedLocationsJsonError")
            {
                DrawTextEx(fontDefault,"there was error while trying to open savedLocations.json",Vector2({0,0}),50,1,BLACK);
                DrawTextEx(fontDefault,"please repair it yourself or delete it and edit values in",Vector2({0,50}),50,1,BLACK);
                DrawTextEx(fontDefault,"new file that will generate after opening app without file",Vector2({0,100}),50,1,BLACK);
                
                for(Button i : buttonsVector)
                {
                    i.Draw();
                    if(i.IsClicked())
                    {
                        if(i.text == "close app")
                        {
                            exit(1);
                        }
                        if(i.text == "try again")
                        {
                            buttonsVector.clear();
                            state = "savedStart";
                        }
                        if(i.text == "run without saved")
                        {
                            buttonsVector.clear();
                            savedAllowed = false;
                            state = "start";
                        }
                        
                    }
                }

            }
            

            

        EndDrawing();   
    }
    CloseWindow();     

    

    
    

    
   
    
    
        
    
    
    
    
    
    

    

    
    if (state != "openingSavedLocationsJsonError")
    {
    savedLocationsFile.open("savedLocations.json",std::fstream::in | std::fstream::out |std::fstream::trunc);
    savedLocationsFile<<savedLocationsJson;
    savedLocationsFile.close();
    }
    return 0;
}

