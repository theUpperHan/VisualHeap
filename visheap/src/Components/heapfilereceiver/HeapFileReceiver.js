import { useEffect, useState } from 'react'
import { Link } from "react-router-dom";
import * as React from 'react';

import Box from '@mui/material/Box';
import Button from '@mui/material/Button';
import Input from '@mui/material/Input'


export default function HeapFileReceiver() {
    const [ fileContent, setFileContent ] = useState(null);
    
    const handleFileUpload = (event) => {
        const file = event.target.files[0];
        if (file && file.type === "application/json") {
            console.log('get file');
            const reader = new FileReader();
            reader.onload = (e) => {
                try {
                    const json = JSON.parse(e.target.result);
                    setFileContent(json);
                    console.log(json);
                } catch (err) {
                    console.error("Error Parsing JSON: ", err)
                }
            }

            reader.readAsText(file);
        }
    };

    function buildSingleBlock(item) {
        return (
            <Box key={item.address}>
                Address: {item.address}, Size: {item.size}
            </Box>
        );
    }

    return(
        <Box>
            <Input type='file' accept='.json' id='heapJson' onChange={handleFileUpload}/>
            {
                fileContent === null ?
                (<div>NO FILE</div>) :
                (fileContent.map((item) => buildSingleBlock(item)))
            }
        </Box>
    );
}