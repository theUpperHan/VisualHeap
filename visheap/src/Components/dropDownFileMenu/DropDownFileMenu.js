import { useState, useEffect } from 'react';
import FolderMonitorReceiver from '../folderMonitorReceiver/FolderMonitorReceiver';
import { Select, MenuItem, Menu } from '@mui/material';
import { fileContentParser } from '../../utils/fileUtils';

export default function DropDownFileMenu() {
    const fileData = FolderMonitorReceiver();
    const [allFiles, setAllFiles] = useState();
    const [fileNames, setFileNames] = useState();
    const [selectedFileContent, setSelectFileContent] = useState('');
    const [selectedFileName, setSelectFileName] = useState('');
    useEffect(() => {
        console.log(fileData);
        setAllFiles(fileData);
        setFileNames(Object.keys(fileData));

    }, [fileData]);

    const handleFileChange = (event) => {
        const selectedFile = event.target.value;
        setSelectFileName(selectedFile);
        setSelectFileContent(allFiles[selectedFile]);

        // console.log(selectedFile);
        // console.log(allFiles[selectedFile]);
        // console.log(selectedFileContent);
    };

    return (
        <div>
            <Select value={selectedFileName} onChange={handleFileChange} sx={{width: '100%'}}>
                <MenuItem value="" disabled>
                    Select a file
                </MenuItem>
                {fileNames ? 
                    fileNames.map((fileName) => (
                        <MenuItem key={fileName} value={fileName}>
                            {fileName}
                        </MenuItem>
                    ))
                    :
                    <div></div>
                }
            </Select>
        </div>
    );
}