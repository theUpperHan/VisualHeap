import { useState, useEffect } from 'react';
import FolderMonitorReceiver from '../folderMonitorReceiver/FolderMonitorReceiver';
import { Select, MenuItem, Menu, TextField, Button, Grid, Typography } from '@mui/material';
import { fileContentParser } from '../../utils/fileUtils';
import { BinChunkVis, BinChunkVisPopulator } from '../visualizers/HeapVisualizer';
import { all } from 'axios';

function populateMenu(fileData) {
    
}

export default function FileSelection() {
    const fileData = FolderMonitorReceiver();
    const [allFiles, setAllFiles] = useState();
    const [fileName, setFileName] = useState('');
    const [fileExe, setFileExe] = useState('');
    const [fileHeapInfo, setFileHeapInfo] = useState();
    const [displayFileName, setDisplayFileName] = useState('');

    useEffect(() => {
        setAllFiles(fileData);
    }, [fileData]);

    const handleFileUpload = (event) => {
        const file = event.target.files[0];
        const reader = new FileReader();

        if (file.name == undefined) {return;} //error catching when cancel the upload window

        // setFileName(file.name);
        console.log(file.name);
        reader.onload = (e) => {
            const heapInfo = fileContentParser(e.target.result);
            setFileHeapInfo(heapInfo);
            setDisplayFileName(file.name);
            console.log(heapInfo.exename);
        };

        reader.readAsText(file);
    };

    const handleFileSelection = (event) => {
        const selectedFile = event.target.value;
        // console.log(selectedFile);
        // console.log(allFiles[selectedFile]);
        setFileName(selectedFile);
        setDisplayFileName(selectedFile.split('/').slice(-1));
        // console.log(selectedFile);
        // console.log(allFiles[selectedFile]);
        const heapInfo = fileContentParser(allFiles[selectedFile]);
        console.log(heapInfo.exename);

        setFileHeapInfo(heapInfo);
    };

    



    return (
        <div>
            <Grid container columns={1} sx={{width: '100%'}}>
                
                <Grid item xs={1}>
                    <Typography variant="h4" gutterBottom>
                        {displayFileName ? displayFileName : "Select a file to view"}
                    </Typography>
                </Grid>
                <Grid item xs={1}>
                    <Select value={fileName} onChange={handleFileSelection} sx={{width: '50%'}}>
                        <MenuItem vale="" disabled>Select a file</MenuItem>
                        {allFiles ? 
                            Object.keys(allFiles).map((fileName) => (
                                <MenuItem key={fileName} value={fileName}>
                                    {fileName}
                                </MenuItem>
                            ))
                            :
                            <div></div>
                        }
                    </Select>
                </Grid>
                <Grid item xs={1}>
                    <TextField accept='.txt' style={{display: 'none'}} id='file-receiver' type='file' onChange={handleFileUpload}/>
                    <label htmlFor='file-receiver'>
                        <Button variant='contained' component='span'>Upload</Button>
                    </label>
                </Grid>
                
            </Grid>

            

            

            {fileHeapInfo ? <BinChunkVisPopulator binChunks={fileHeapInfo.heaps[0].regularbins}/> : <div>Nothing</div>}



        </div>
    );
}