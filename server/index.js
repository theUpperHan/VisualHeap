const express = require('express');
const cors = require('cors');
const http = require('http');
const socketIO = require('socket.io');
const chokidar = require('chokidar');
const fs = require('fs');
const path = require('path');
const { rejects } = require('assert');

const app = express();
app.use(cors({
    origin: 'http://localhost:3000',
}));
const server = http.createServer(app);
const io = socketIO(server, {
    cors: {
        origin: 'http://localhost:3000',
        method: ['GET', 'POST'],
    },
});


const folderToWatch = '/home/uprhan/heapInfo'

const watcher = chokidar.watch(folderToWatch, {
    ignored: /(^|[\/\\])\../,
    persistent: true,
    interval: 10000,
});


// Folder watcher
watcher.on('change', (path) => {
    console.log(`File ${path} has been changed`);

    fs.readFile(path, 'utf-8', (err, data) => {
        if (err) {
            console.error('Error reading file', err);
            return;
        }

        io.emit('fileChange', {
            path: path,
            data: data
        });
    });
});

watcher.on('add', (path) => {
    console.log(`File ${path} has been added`);
    

    fs.readFile(path, 'utf-8', (err, data) => {
        if (err) {
            console.error('Error reading file', err);
            return;
        }

        io.emit('fileCreate', {
            path: path,
            data: data
        });
    });
});

watcher.on('unlink', (path) => {
    console.log(`File ${path} has been deleted`);
    io.emit('fileDelete', {path});
})


// Server socket listens to client emits
io.on('connection', (socket) => {
    console.log('Client connected');

    socket.on('disconnect', () => {
        console.log('Client disconnected');
    });

    socket.on('getExistingFiles', () => {    
        // Recursive function to read files in a directory and its subdirectories
        const readFilesRecursively = (dirPath) => {
            return new Promise((resolve, reject) => {
                fs.readdir(dirPath, { withFileTypes: true }, (err, entries) => {
                    if (err) {
                        reject(err);
                    } else {
                        const fileDataPromises = entries.map((entry) => {
                            const fullPath = path.join(dirPath, entry.name);
                            if (entry.isDirectory()) {
                                return readFilesRecursively(fullPath);
                            } else {
                                return new Promise((resolve, reject) => {
                                    fs.readFile(fullPath, 'utf8', (err, data) => {
                                        if (err) {
                                            reject(err);
                                        } else {
                                            resolve({ path: fullPath, data });
                                        }
                                    });
                                });
                            }
                        });

                        Promise.all(fileDataPromises)
                            .then((fileDataArray) => {
                                const flattenedFileData = fileDataArray.flat();
                                resolve(flattenedFileData);
                            })
                            .catch((err) => {
                                reject(err);
                            });
                    }
                });
            });
        };
    
        // Read files recursively starting from the folderPath
        readFilesRecursively(folderToWatch)
            .then((fileDataArray) => {
                const fileData = fileDataArray.reduce((acc, { path, data }) => {
                    acc[path] = data;
                    return acc;
                }, {});
                socket.emit('existingFileData', fileData);
            })
            .catch((err) => {
                console.error('Error reading files:', err);
            });
    });
    


});

server.listen(8080, () => {
    console.log('server listening on port 8080')
});

app.get('/', (req, res) => {
    res.send('Hello from server');
})
