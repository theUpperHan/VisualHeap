import './App.css';

import axios from 'axios';
import io from 'socket.io-client'
import { Route, Routes } from 'react-router-dom';
import { useEffect, useState } from 'react';
import VisHeapNavBar from './Components/visHeapNavBar/VisHeapNavBar'
import FolderMonitorReceiver from './Components/folderMonitorReceiver/FolderMonitorReceiver';
import FileReceiver from './Components/fileUploader/FileUploader';
import FileSelection from './Components/fileSelection/FileSelection';

const apiCall = () => {
    axios.get('http://localhost:8080').then((data) => {
        console.log(data)
    })
}

function App() {

    return (
        <div className="App">

            <VisHeapNavBar />
            {/* <button onClick={apiCall}>Make API call</button> */}
            {/* <FileReceiver/> */}
            <FileSelection/>
        </div>
    );
}


export default App;
