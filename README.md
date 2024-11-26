## Green Pass Management System

This project implements a **Green Pass management service**, designed to ensure the validity of health certificates.

### Workflow:
1. **User**: After vaccination, submits their health card code to the vaccination center via a client.
2. **Vaccination Center**: The center forwards the received data to **ServerV**, which stores the information in an **SQLite** database.
3. **ClientS**: Requests certificate validation from **ServerG**, which checks the validity by querying **ServerV**.
4. **ClientT**: Authorized to invalidate or restore the Green Pass based on infection or recovery, by updating the status via **ServerG**.

### Key Features:
- **Secure Data Storage**: User and certificate data are stored safely in an SQLite database.
- **Real-time Validity Checks**: Validates Green Pass status based on the health card code.
- **Admin Privileges**: Allows authorized clients to modify the validity of a Green Pass.
