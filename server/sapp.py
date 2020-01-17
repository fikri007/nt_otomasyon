""" 
    BİLGİ: Nt Otomasyon İçin Şablon
"""
import os
from sanic import Sanic
from sanic import response
import socketio
from sanic_auth import Auth
from sanic_mongodb_ext import MongoDbExtension
from umongo import Instance, Document, BaseSchema, pre_dump, MotorAsyncIOInstance
from umongo.fields import StringField,IntegerField
import logging
import pymongo
from motor.motor_asyncio import AsyncIOMotorClient


uri = "mongodb://127.0.0.1:27017"
# vt = pymongo.MongoClient(uri)
# BİLGİ:  Sanic App Oluşturuluyor

app = Sanic('motor_mongodb')
app.config.update({
    "MONGODB_DATABASE": "fikoyt", # Make ensure that the `app` database is really exists
    "MONGODB_URI": "mongodb://localhost:27017",
    "LAZY_UMONGO": MotorAsyncIOInstance(),
    "REQUEST_MAX_SIZE":1073741824
})
app.websocket_enabled=True



# AYRAÇ: Eklenti Tanımları Bölümü
# BİLGİ: Mongo Db Tanımalnıyor
MongoDbExtension(app)
# NOT: Socketio Oluşturuluyor
sio = socketio.AsyncServer(async_mode='sanic',  engineio=logging.getLogger(__name__), logger=True, engineio_logger=True)
sio.transports = ["websocket"]
sio.namespace="/"
sio.attach(app)
# NOT: Giriş Kontrolü İçin uth İntegrate Ediliyor
auth = Auth(app)

# AYRAÇ: Sezon Tanımı
session = {}
@app.middleware('request')
async def add_session(request):
    print(request.headers.get("FikoYT-SSL-Security"))
    request['session'] = session


# AYRAÇ: Mongodb Örnek Şablon

#  BİLGİ:  Model Örneği
@app.lazy_umongo.register
class Artist(Document):
    name = StringField(required=True, allow_none=False)
    toplam_sayi = IntegerField()

# AYRAÇ: Http Server Ayarları

 # BİLGİ:  Kök Url
@app.route("/")
async def handle(request):
    """ Kök Url """
    artist = Artist(name="A new rockstar!",toplam_sayi=await app.vt.fikoyt.artist.count_documents({}))
    await artist.commit()
    data = await app.vt.fikoyt.artist.find().to_list(5000)
    for x in data:
        x['id'] = str(x['_id'])
        del x['_id']
    
    return response.json(data)

# BİLGİ:  Server Başlamadan Önce 
@app.listener('before_server_start')
def init(sanic, loop):
    app.vt = AsyncIOMotorClient(uri)

# AYRAÇ: Soketio Ayarları

# BİLGİ:  Socketio İsteği Sonlandığında Http
@sio.event
async def disconnect_request(sid):
    await sio.disconnect(sid)

global count
count=0
# BİLGİ: Bir Socket Açıldığında
@sio.event
async def connect(sid, environ):
    global count
    count+=1
    print('Client Bağlandı',sid, "Socket Sayısı->", count, sio.namespace)


@sio.event
def disconnect(sid):
    global count
    count-=1
    print('Client Koptu', sid, "Socket Sayısı->", count, sio.namespace)

# @sio.on("fikoSistem.miGirisYapilmis1", namespace="/")
# def asd(*args,**kwargs):
#     print("sio.onnnnnnnnnnnnnnnnnnnn")
#     return False
    

# AYRAÇ: Deneme


# AYRAÇ: Ayarlar Ve Çalıştırma

# NOT:  Sertifikalar Tanımlanıyor Şimdilik Gerek Yok
# CERT_FILE = os.path.abspath(os.path.join(os.path.dirname(
#     __file__), ".", "sertifika", "server.crt"))  # "../sertifika/server.crt""
# KEY_FILE = os.path.abspath(os.path.join(os.path.dirname(
#     __file__), ".", "sertifika", "server.key"))  # "../sertifika/server.key"

# BİLGİ:  Dosya Çağırıldığı Zaman Server Çalıştırılıyor
if __name__ == "__main__":
    app.run(host='0.0.0.0', port=80, debug=True) #, ssl= {'cert': CERT_FILE, 'key':KEY_FILE}, workers=1)
