from flask import Flask, request, abort
import os

app = Flask(__name__)

def process_json(message):
    contents = message['data']
    contents_bytes = [b for b in bytes.fromhex(contents)]

    image_id = contents_bytes[11]
    seq = contents_bytes[0]
    totseq = contents_bytes[1]

    filename = '{:02d}{:03d}{:03d}.BIN'.format(image_id, seq, totseq)
    with open(filename, 'wb') as f:
        f.write(bytes(contents_bytes))



@app.route('/webhook', methods=['POST'])
def webhook():
    if request.method == 'POST':
        contents = request.json
        # with concurrency could start a thread here - one to return 200 message, one to process message (to avoid waiting)
        process_json(contents)
        return 'success', 200
    else:
        abort(400)

if __name__ == '__main__':
    app.run()