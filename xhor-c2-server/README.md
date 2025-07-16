# Setup

1. Install the dependencies using `npm install`
2. Start the server using `npm start`
3. Test the server:

```sh
curl -X POST http://localhost:3000/implant/checkin -H "Content-Type: application/json" -d '{"hostname": "WSLVM", "os": "linux"}'
```

---

# Usage

- The operator page is accessible via `http://localhost:3000/operator.html`. You can use this interface to manage your agents and send/receive commands