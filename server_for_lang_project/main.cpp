/*
 Вставить в начальные параметры CMake:
 -DCMAKE_TOOLCHAIN_FILE=/home/lost_brain/vcpkg/scripts/buildsystems/vcpkg.cmake
*/


#include <iostream>
#include <algorithm>

#include "uwebsockets/App.h"

using namespace std;



//В ЗАГОЛОВКИ:

 // hardware_concurrency - макс число "физических" потоков в процессоре
vector<thread *> srvThreads(thread::hardware_concurrency());

short int port = 3000; // номер порта сервера

 //Карта всех подключенных пользователей
map<unsigned long int, string> online_users;


 // Какую информацию о пользователе мы храним
struct UserConnections {
    string user_name; //ПОПРОБОВАТЬ УДАЛИТЬ
    unsigned long int user_id;
};



 //Вынос в функцию кода из .message
void newUserConnected(string channel, uWS::WebSocket<true, true, UserConnections> *ws, string name = "", unsigned long int id = 0)
{

    cout << "Test call newUserConnected is succes! " << name << endl;

    //string_view testmesage("TEST MESSAGE TO NEW USER");
    //ws->publish(channel, testmesage);
}





int main() {


     // atomic_ulong - атомарный тип данных, подходит для изменения из разных потоков
    atomic_ulong latest_user_id = 1; // порядковй номер подключенного пользователя

    // //Выполним действие для каждого элемента вектора srvThreads - создание сервера в каждом потоке
    //transform(srvThreads.begin(), srvThreads.end(), srvThreads.begin(), [&latest_user_id](thread *thr){
    //    return new thread([&latest_user_id](){

            // Настраиваем сервер /* - означает локальный адрес
           uWS::SSLApp({
                 .key_file_name = "../misc/key.pem",
                 .cert_file_name = "../misc/cert.pem",
                 .passphrase = "1234"

           }).ws<UserConnections>("/*", {

                   // вызывается при новом подключении, точка означает, что open - структура
                 .open = [&latest_user_id](auto *ws)
                 {



                     UserConnections *data = static_cast<UserConnections *>(ws->getUserData()); // ПРОВЕРИТЬ ОЧИСТКУ ПАМЯТИ
                     data->user_id = latest_user_id++;
                     data->user_name = "UNNAMED";

                      // подписали всех пользователей на канал broadcast для расылки уведомлений
                     ws->subscribe("broadcast");
                      // подписываем каждого пользователя на его личный канал для приема личных сообщений
                     string user_channel = "user#" + to_string(data->user_id);
                     ws->subscribe(user_channel);
                      //Поумолчанию задем имя "UNNAMED"

                     cout << "New user connected, id: " << data->user_id << " THREAD_ID: " << this_thread::get_id() << " USER_CHANNEL: " << user_channel << endl;

                     for(auto entry : online_users)
                     {
                           //Так можно отправить новому пользователю список всех подключившихся пользователей
                         newUserConnected(user_channel, ws, entry.second, entry.first);
                     }

                     online_users[data->user_id] = data->user_name;

                     //newUserConnected(user_channel, ws, "open", 1);
                     //ws->send("Message sent", uWS::OpCode::TEXT);

                     string_view testmesage(R"(TEST MESSAGE TO NEW USER)");
                     ws->publish(user_channel, testmesage, uWS::OpCode::TEXT, false);
                 },

                   // вызывается при каждом получении нового сообщения
                 .message = [](auto *ws, string_view message, uWS::OpCode opCode)
                 {
                    UserConnections *data = static_cast<UserConnections *>(ws->getUserData()); // ПРОВЕРИТЬ ОЧИСТКУ ПАМЯТИ
                    string_view beginning = message.substr(0,10); // УЯЗВИМОСТЬ



                    cout << "The received message: " << message << endl;
                    string_view testmesage(R"(TEST MESSAGE TO NEW USER)");
                    ws->publish("user#1", testmesage);



                    if(beginning.compare("USER_NAME=") == 0) // УЯЗВИМОСТЬ
                    {
                        string_view name = message.substr(10); // УЯЗВИМОСТЬ
                        data->user_name = static_cast<string>(name);
                        string broadcast_message = "NEW_USER," + data->user_name + "," + to_string(data->user_id);
                        ws->publish("broadcast", static_cast<string_view>(broadcast_message), opCode, false); //Уведомление всех пользователей о подключении нового участника
                        online_users[data->user_id] = data->user_name;
                        newUserConnected("broadcast", ws, data->user_name, data->user_id); //Вызов тестовой функции
                    }

                    if(!message.substr(0, 11).compare("MESSAGE_TO,"))
                    {
                        short int comma_pos = message.substr(11).find(',');
                        string_view usr_id = message.substr(11, comma_pos);
                        string_view mess_body = message.substr(comma_pos + 1);
                        cout << "MESS_DEB: " << usr_id << " " << mess_body << endl;
                        ws->publish("user#" + static_cast<string>(usr_id), mess_body, uWS::OpCode::TEXT, false);

                        string_view testmesage(R"(TEST MESSAGE TO NEW USER)");
                        ws->publish("user#" + static_cast<string>(usr_id), testmesage);
                    }

                 },
                   //Вызывается автоматически при отключении полььзователя
                 .close = [](auto *ws, int code, string_view message)
                 {
                        UserConnections *data = static_cast<UserConnections *>(ws->getUserData()); // ПРОВЕРИТЬ ОЧИСТКУ ПАМЯТИ
                        ws->publish("broadcast", "DISCONNECT=" + to_string(data->user_id));
                        online_users.erase(data->user_id);
                 },

           }).listen(port, [](auto *token) { // listen выполняется при старте приложения

                 if (token)
                 {
                     cout << "Listening on port " << port << endl;
                 }

           }).run();
    //    });
    //});


   // //Завершение работы потоков
   //for_each(srvThreads.begin(), srvThreads.end(), [](thread *thr){
   //    thr->join();
   //});



    return 0;
}
