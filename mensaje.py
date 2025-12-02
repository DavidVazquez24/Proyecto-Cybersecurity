import asyncio
from telegram import Bot

bot_token = "8587312904:AAE-4k1CM9cPDmDa2JOdi98Z0kO7VJydAQE"
chat_id_code = 5746177016

bot = Bot(token=bot_token)
def mensajeTG(mensaje1,mensaje2):
    async def enviarSinResponder(textoMensaje):
        await bot.send_message(chat_id=chat_id_code, text=textoMensaje)

    asyncio.run(enviarSinResponder(mensaje1+"\n"+mensaje2))
