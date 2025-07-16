import express from 'express';
import bodyParser from 'body-parser';
import implantRoutes from './routes/implant.js';
import operatorRoutes from './routes/operator.js';

const app = express();
const PORT = 3000;

app.use(bodyParser.json());
app.use('/implant', implantRoutes);
app.use('/operator', operatorRoutes);
app.use(express.static('public'));


app.listen(PORT, () => {
  console.log(`C2 server listening on http://localhost:${PORT}`);
});

