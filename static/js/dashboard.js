
document.addEventListener('DOMContentLoaded', () => {
    showPage('home');
    const defaultSidebarLink = document.querySelector('.sidebar ul li a');
    if (defaultSidebarLink) defaultSidebarLink.classList.add('active');
});

function setActive(selected) {
    document.querySelectorAll('.sidebar ul li a').forEach(a => a.classList.remove('active'));
    selected.classList.add('active');
}

function formatTimestamp(timestamp) {
    const date = new Date(timestamp);
    return date.toLocaleString('en-GB', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit'
    }).replace(',', '');
}

function showPage(page) {
    document.querySelectorAll('.page').forEach(p => p.style.display = 'none');
    document.getElementById(`${page}-page`).style.display = 'block';
}

fetch('/get_hits_data')
    .then(response => response.json())
    .then(dataFromFlask => {
        const tableBody = document.getElementById('data-table');
        tableBody.innerHTML = '';
        let totalHits = 0;
        dataFromFlask.forEach((item, index) => {
            const row = tableBody.insertRow();
            row.insertCell(0).textContent = index + 1;
            row.insertCell(1).textContent = item.hit;
            row.insertCell(2).textContent = formatTimestamp(item.timestamp);
            if (item.hit === "detected") totalHits++;
        });
        document.getElementById('hit-count').textContent = totalHits;
    })
    .catch(error => console.error('Error fetching hits data:', error));

fetch('/get_temperature_data')
.then(response => response.json())
.then(dataFromFlask => {
    const tableBody = document.getElementById('temperature-table');
    tableBody.innerHTML = '';
    const values = [];
    const timestamps = [];

    // Urutkan data berdasarkan timestamp secara menurun
    dataFromFlask.sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp));

    // Ambil data suhu terbaru (data pertama setelah diurutkan)
    const latesttemperature = dataFromFlask[0];

    // Update elemen latest-temperature
    document.getElementById('latest-temperature').textContent = latesttemperature.temperature;

    // Proses untuk menampilkan data dalam tabel dan grafik
    dataFromFlask.forEach((item, index) => {
        const row = tableBody.insertRow();
        row.insertCell(0).textContent = index + 1;
        row.insertCell(1).textContent = item.temperature;
        row.insertCell(2).textContent = formatTimestamp(item.timestamp);
        values.push(item.temperature);
        timestamps.push(formatTimestamp(item.timestamp));
    });

    const ctx = document.getElementById('temperatureChart').getContext('2d');
    new Chart(ctx, {
        type: 'line',
        data: {
            labels: timestamps,
            datasets: [{
                label: 'temperature Over Time',
                data: values,
                borderColor: 'rgba(75, 192, 192, 1)',
                borderWidth: 2,
                fill: false,
                tension: 0.4,
                pointBackgroundColor: 'rgba(75, 192, 192, 1)',
            }]
        },
        options: {
            responsive: true,
            scales: {
                x: {
                    title: {
                        display: true,
                        text: 'Timestamp'
                    }
                },
                y: {
                    title: {
                        display: true,
                        text: 'temperature Value'
                    },
                    min: 20, // Minimum value for the y-axis
                    max: 30, // Maximum value for the y-axis
                    ticks: {
                        stepSize: 1, // Ensure each tick represents a step of 1
                        callback: function(value) {
                            if (value >= 20 && value <= 30) {
                                return value; // Only show labels from 20 to 30
                            }
                            return null; // Hide other labels
                        }
                    }
                }
            }
        }
    });


})
.catch(error => console.error('Error fetching temperature data:', error));

fetch('/get_humidity_data')
.then(response => response.json())
.then(dataFromFlask => {
    const tableBody = document.getElementById('humidity-table');
    tableBody.innerHTML = '';
    const values = [];
    const timestamps = [];

    // Urutkan data berdasarkan timestamp secara menurun
    dataFromFlask.sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp));

    // Ambil data suhu terbaru (data pertama setelah diurutkan)
    const latesthumidity = dataFromFlask[0];

    // Update elemen latest-humidity
    document.getElementById('latest-humidity').textContent = latesthumidity.humidity;

    // Proses untuk menampilkan data dalam tabel dan grafik
    dataFromFlask.forEach((item, index) => {
        const row = tableBody.insertRow();
        row.insertCell(0).textContent = index + 1;
        row.insertCell(1).textContent = item.humidity;
        row.insertCell(2).textContent = formatTimestamp(item.timestamp);
        values.push(item.humidity);
        timestamps.push(formatTimestamp(item.timestamp));
    });

    const ctx = document.getElementById('humidityChart').getContext('2d');
    new Chart(ctx, {
        type: 'line',
        data: {
            labels: timestamps,
            datasets: [{
                label: 'humidity Over Time',
                data: values,
                borderColor: 'rgba(75, 192, 192, 1)',
                borderWidth: 2,
                fill: false,
                tension: 0.4,
                pointBackgroundColor: 'rgba(75, 192, 192, 1)',
            }]
        },
        options: {
            responsive: true,
            scales: {
                x: {
                    title: {
                        display: true,
                        text: 'Timestamp'
                    }
                },
                y: {
                    title: {
                        display: true,
                        text: 'humidity Value'
                    },
                    min: 60, // Minimum value for the y-axis
                    max: 90, // Maximum value for the y-axis
                    ticks: {
                        stepSize: 1, // Ensure each tick represents a step of 1
                        callback: function(value) {
                            if (value >= 60 && value <= 90) {
                                return value; // Only show labels from 20 to 30
                            }
                            return null; // Hide other labels
                        }
                    }
                }
            }
        }
    });

    
})
.catch(error => console.error('Error fetching humidity data:', error));
