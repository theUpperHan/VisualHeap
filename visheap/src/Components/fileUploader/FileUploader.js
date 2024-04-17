import React, {useState} from 'react';
import { Button, TextField, Typography } from '@mui/material';
import { fileContentParser } from '../../utils/fileUtils';
import { BinChunkVis, BinChunkVisPopulator } from '../visualizers/HeapVisualizer';

export default function FileReceiver() {
    const [fileName, setFileName] = useState('');
    const [fileContent, setFileContent] = useState('');
    const [fileHeapInfo, setFileHeapInfo] = useState();

    const handleFileUpload = (event) => {
        const file = event.target.files[0];
        const reader = new FileReader();
        
        if (file.name === undefined) {
            return;
        }

        setFileName(file.name);
        reader.onload = (e) => {
            setFileHeapInfo(fileContentParser(e.target.result));
            setFileContent(e.target.result);
        };


        reader.readAsText(file);
    };

    return (
        <div>
            <input
                accept='.txt'
                style={{display: 'none'}}
                id="file-receiver"
                type='file'
                onChange={handleFileUpload}
            />
            <label htmlFor='file-receiver'>
                <Button variant='contained' component='span'>
                    Upload
                </Button>
            </label>
            <TextField
                multiline
                rows={10}
                fullWidth
                variant="outlined"
                value={fileContent}
                InputProps={{
                readOnly: true,
                }}
                sx={{ mt: 2 }}
            />

            {/* {fileHeapInfo ? <BinChunkVis binChunk={fileHeapInfo.heaps[0].regularbins[0]}/> : <div>Nothing</div>} */}
            {fileHeapInfo ? <BinChunkVisPopulator binChunks={fileHeapInfo.heaps[0].regularbins}/> : <div>Nothing</div>}

        </div>
    )

}