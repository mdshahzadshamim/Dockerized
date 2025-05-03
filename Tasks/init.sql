CREATE TABLE IF NOT EXISTS list (
    id BIGSERIAL PRIMARY KEY NOT NULL,
    subject_name VARCHAR(50) NOT NULL,
    topic VARCHAR(50) NOT NULL,
    study_date DATE NOT NULL DEFAULT CURRENT_DATE,
    study_type INT CHECK (study_type BETWEEN 0 AND 7),
    completion_status BOOLEAN DEFAULT FALSE,
    revision_rating INT CHECK (revision_rating BETWEEN 0 AND 3)
);