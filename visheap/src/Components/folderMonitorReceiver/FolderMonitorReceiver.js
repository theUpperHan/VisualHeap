import { useEffect, useState } from 'react';
import io from 'socket.io-client'


export default function FolderMonitorReceiver() {
    const [fileData, setFileData] = useState({});

    useEffect(() => {
        const socket = io('http://localhost:8080', {
            transports: ['websocket'],
        });
        
        socket.on('connect_error', (error) => {
            console.error('Socket connection error:', error);
        });

        socket.emit('getExistingFiles');

        socket.on('existingFileData', (data)=> {
            setFileData(data);
        });

        socket.on('fileCreate', (data) => {
            setFileData((prevFileData) => ({
                ...prevFileData,
                [data.path]: data.data,
            }));
        });


        socket.on('fileChange', (data) => {
            setFileData((prevFileData) => ({
                ...prevFileData,
                [data.path]: data.data,
            }));
        });

    }, []);

    // useEffect(() => {
    //     console.log('Updated file data:', Object.keys(fileData).length);
    // }, [fileData])
    
    return fileData;
}